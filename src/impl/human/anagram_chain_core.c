/*
 * anagram_chain_core.c - Core algorithm and data structures
 *
 * Contains memory-mode specific implementations:
 * - Dynamic (USE_DYNAMIC_MEMORY): Optimized bulk allocation, fewer malloc calls
 * - Static (default): No malloc/free, all memory pre-allocated at compile time
 *
 * Key optimizations vs AI version:
 * - FNV-1a hash instead of djb2
 * - Counting sort O(n) instead of qsort O(n log n)
 * - Bulk memory allocation (dynamic mode): one malloc for all words/signatures
 * - No visited array in DFS - derived chains can't have cycles
 * - Path buffer sized to max word length, not dict size
 */

#include "global.h"

bool global_trace_enable = false;

#if defined(PLATFORM_ARM)
/* Tick counter for tracing (microseconds since startup) */
volatile unsigned int g_trace_tick_us = 0;

/* Weak function - can be overridden by main_arm.c */
__attribute__((weak)) void trace_update_time(void)
{
#warning "trace_update_time is not implemented. Time will not be recorded."
    UNUSED(g_trace_tick_us);
}
#endif

/*
 * Helper functions
 */

static unsigned long hash_fnv1a(const char *s)
{
    unsigned long h;

    ASSERT_NOT_NULL(s);

    h = FNV_OFFSET_BASIS;

    while (*s)
    {
        h ^= (unsigned char)*s++;
        h *= FNV_PRIME;
    }

    return h;
}

/* Counting sort for signature - O(n) instead of qsort's O(n log n) */
static void sort_chars(char *s, size_t len)
{
    int counts[CHAR_COUNT_SIZE] = {0};
    size_t i;
    size_t pos;
    int c;

    ASSERT_NOT_NULL(s);

    for (i = 0; i < len; i++)
    {
        ASSERT_MSG((unsigned char)s[i] < CHAR_COUNT_SIZE, "char out of range");
        counts[(unsigned char)s[i]]++;
    }

    pos = 0;
    for (c = 0; c < CHAR_COUNT_SIZE; c++)
    {
        while (counts[c]-- > 0)
        {
            s[pos++] = (char)c;
        }
    }
}

/* Insert char into already sorted string, returns new length */
static size_t insert_sorted(char *dst, const char *src, size_t len, char c)
{
    size_t i = 0;
    size_t j = 0;
    int inserted = 0;

    ASSERT_NOT_NULL(dst);
    ASSERT_NOT_NULL(src);

    while (i < len)
    {
        if (!inserted && c < src[i])
        {
            dst[j++] = c;
            inserted = 1;
        }
        else
        {
            dst[j++] = src[i++];
        }
    }

    if (!inserted)
    {
        dst[j++] = c;
    }

    dst[j] = '\0';
    return j;
}

#if defined(USE_DYNAMIC_MEMORY)
/*
 * Uses bulk allocation: one malloc for all words, one for all signatures.
 * This reduces malloc calls from O(2N) to O(1) and improves cache locality.
 */

typedef struct
{
    /* Bulk memory pools for words and signatures */
    char *word_pool;
    char *sig_pool;
    size_t pool_size;
    size_t word_pool_used;
    size_t sig_pool_used;

    /* Current dictionary and hashtable */
    Dictionary *dict;
    HashTable *ht;
    ChainResults *results;

    /* DFS working buffers */
    size_t *dfs_path;
    char dfs_candidate[512];
    char temp_sig[512];
} GlobalState;

static GlobalState G = {0};
#define GLOBAL G

/* DFS search - no visited array needed, chains always grow in length */
static void dfs_dynamic(HashTable *ht, Dictionary *dict, size_t cur,
                        size_t depth)
{
    const char *sig;
    size_t sig_len;
    int found;
    int c;
    HashEntry *entry;
    size_t i;
    size_t next;
    size_t new_cap;
    Chain *new_chains;
    size_t *indices;

    ASSERT_NOT_NULL(ht);
    ASSERT_NOT_NULL(dict);
    ASSERT_MSG(cur < dict->count, "cur index out of bounds");
    ASSERT_MSG(depth <= MAX_CHAIN_DEPTH, "depth exceeds MAX_CHAIN_DEPTH");

    sig = dict->signatures[cur];
    ASSERT_NOT_NULL(sig);
    sig_len = strlen(sig);
    found = 0;

    if (depth >= MAX_CHAIN_DEPTH)
    {
        return;
    }

    /* Try adding each printable ASCII character */
    for (c = ASCII_MIN; c <= ASCII_MAX; c++)
    {
        insert_sorted(GLOBAL.dfs_candidate, sig, sig_len, (char)c);

        entry = hashtable_find(ht, GLOBAL.dfs_candidate);
        if (entry)
        {
            for (i = 0; i < entry->word_count; i++)
            {
                next = entry->word_indices[i];
                found = 1;
                GLOBAL.dfs_path[depth] = next;
                dfs_dynamic(ht, dict, next, depth + 1);
            }
        }
    }

    /* Leaf node - save chain if it's the longest found so far */
    if (!found && GLOBAL.results)
    {
        if (depth > GLOBAL.results->max_length)
        {
            /* New longest chain - clear previous results */
            for (i = 0; i < GLOBAL.results->count; i++)
            {
                free(GLOBAL.results->chains[i].indices);
            }
            GLOBAL.results->count = 0;
            GLOBAL.results->max_length = depth;
        }

        if (depth == GLOBAL.results->max_length &&
            GLOBAL.results->count < MAX_CHAINS)
        {
            /* Grow chains array if needed */
            if (GLOBAL.results->count >= GLOBAL.results->capacity)
            {
                new_cap = GLOBAL.results->capacity * 2;
                new_chains =
                    realloc(GLOBAL.results->chains, new_cap * sizeof(Chain));
                if (!new_chains)
                {
                    return;
                }
                GLOBAL.results->chains = new_chains;
                GLOBAL.results->capacity = new_cap;
            }

            /* Store this chain */
            indices = malloc(depth * sizeof(size_t));
            if (!indices)
            {
                return;
            }
            memcpy(indices, GLOBAL.dfs_path, depth * sizeof(size_t));
            GLOBAL.results->chains[GLOBAL.results->count].indices = indices;
            GLOBAL.results->chains[GLOBAL.results->count].length = depth;
            GLOBAL.results->count++;
        }
    }
}

Dictionary *dictionary_create(size_t cap)
{
    Dictionary *dict = NULL;
    size_t pool_size;
    int success = 0;

    TRACE(">> dictionary_create cap=%u", (unsigned)cap);

    do
    {
        dict = malloc(sizeof(Dictionary));
        if (!dict)
        {
            break;
        }

        dict->words = malloc(cap * sizeof(char *));
        dict->signatures = malloc(cap * sizeof(char *));
        if (!dict->words || !dict->signatures)
        {
            break;
        }

        /* Pre-allocate bulk pools - estimate 12 bytes per word average */
        pool_size = cap * 24;
        GLOBAL.word_pool = malloc(pool_size);
        GLOBAL.sig_pool = malloc(pool_size);
        if (!GLOBAL.word_pool || !GLOBAL.sig_pool)
        {
            break;
        }

        GLOBAL.pool_size = pool_size;
        GLOBAL.word_pool_used = 0;
        GLOBAL.sig_pool_used = 0;

        dict->count = 0;
        dict->capacity = cap;
        GLOBAL.dict = dict;

        success = 1;
    } while (0);

    if (!success)
    {
        if (dict)
        {
            free(dict->words);
            free(dict->signatures);
            free(dict);
        }
        free(GLOBAL.word_pool);
        free(GLOBAL.sig_pool);
        GLOBAL.word_pool = NULL;
        GLOBAL.sig_pool = NULL;

        TRACE("<< dictionary_create (failed)");

        return NULL;
    }

    TRACE("<< dictionary_create");

    return dict;
}

void dictionary_free(Dictionary *dict)
{
    TRACE(">> dictionary_free");

    if (!dict)
    {
        TRACE("<< dictionary_free (NULL)");
        return;
    }

    /* Single free for each pool instead of N frees */
    free(GLOBAL.word_pool);
    free(GLOBAL.sig_pool);
    GLOBAL.word_pool = NULL;
    GLOBAL.sig_pool = NULL;
    GLOBAL.pool_size = 0;
    GLOBAL.word_pool_used = 0;
    GLOBAL.sig_pool_used = 0;

    free(dict->words);
    free(dict->signatures);
    free(dict);
    GLOBAL.dict = NULL;

    TRACE("<< dictionary_free");
}

int dictionary_add(Dictionary *dict, const char *word)
{
    size_t new_cap;
    char **new_words;
    char **new_sigs;
    size_t len;
    size_t needed;
    size_t new_size;
    uintptr_t old_word_base;
    uintptr_t old_sig_base;
    char *new_word_pool;
    char *new_sig_pool;
    ptrdiff_t word_delta;
    ptrdiff_t sig_delta;
    size_t i;
    char *word_ptr;
    char *sig_ptr;

    TRACE(">> dictionary_add word=%s", word ? word : "NULL");

    if (!dict || !word)
    {
        TRACE("<< dictionary_add (invalid args)");
        return -1;
    }

    /* Grow pointer arrays if needed */
    if (dict->count >= dict->capacity)
    {
        new_cap = dict->capacity * 2;
        new_words = realloc(dict->words, new_cap * sizeof(char *));
        new_sigs = realloc(dict->signatures, new_cap * sizeof(char *));
        if (!new_words || !new_sigs)
        {
            TRACE("<< dictionary_add (realloc failed)");
            return -1;
        }
        dict->words = new_words;
        dict->signatures = new_sigs;
        dict->capacity = new_cap;
    }

    len = strlen(word);
    needed = len + 1;

    /* Grow pools if needed */
    if (GLOBAL.word_pool_used + needed > GLOBAL.pool_size ||
        GLOBAL.sig_pool_used + needed > GLOBAL.pool_size)
    {
        new_size = GLOBAL.pool_size * 2;

        /* Save old base addresses for pointer adjustment */
        old_word_base = (uintptr_t)GLOBAL.word_pool;
        old_sig_base = (uintptr_t)GLOBAL.sig_pool;

        new_word_pool = realloc(GLOBAL.word_pool, new_size);
        new_sig_pool = realloc(GLOBAL.sig_pool, new_size);
        if (!new_word_pool || !new_sig_pool)
        {
            TRACE("<< dictionary_add (pool realloc failed)");
            return -1;
        }

        /* Adjust existing pointers to new pool locations */
        word_delta = (uintptr_t)new_word_pool - old_word_base;
        sig_delta = (uintptr_t)new_sig_pool - old_sig_base;
        for (i = 0; i < dict->count; i++)
        {
            dict->words[i] += word_delta;
            dict->signatures[i] += sig_delta;
        }

        GLOBAL.word_pool = new_word_pool;
        GLOBAL.sig_pool = new_sig_pool;
        GLOBAL.pool_size = new_size;
    }

    /* Store word in pool */
    word_ptr = GLOBAL.word_pool + GLOBAL.word_pool_used;
    memcpy(word_ptr, word, len + 1);
    GLOBAL.word_pool_used += len + 1;
    dict->words[dict->count] = word_ptr;

    /* Compute and store signature in pool */
    sig_ptr = GLOBAL.sig_pool + GLOBAL.sig_pool_used;
    memcpy(sig_ptr, word, len + 1);
    sort_chars(sig_ptr, len);
    GLOBAL.sig_pool_used += len + 1;
    dict->signatures[dict->count] = sig_ptr;

    dict->count++;

    TRACE("<< dictionary_add count=%u", (unsigned)dict->count);

    return 0;
}

HashTable *hashtable_create(size_t bucket_count)
{
    HashTable *ht = NULL;
    int success = 0;

    TRACE(">> hashtable_create");

    do
    {
        ht = malloc(sizeof(HashTable));
        if (!ht)
        {
            break;
        }

        ht->buckets = calloc(bucket_count, sizeof(HashEntry *));
        if (!ht->buckets)
        {
            break;
        }

        ht->bucket_count = bucket_count;
        ht->entry_count = 0;
        GLOBAL.ht = ht;

        success = 1;
    } while (0);

    if (!success)
    {
        if (ht)
        {
            free(ht);
        }

        TRACE("<< hashtable_create (failed)");

        return NULL;
    }

    TRACE("<< hashtable_create");

    return ht;
}

void hashtable_free(HashTable *ht)
{
    size_t i;
    HashEntry *entry;
    HashEntry *next;

    TRACE(">> hashtable_free");

    if (!ht)
    {
        TRACE("<< hashtable_free (NULL)");
        return;
    }

    for (i = 0; i < ht->bucket_count; i++)
    {
        entry = ht->buckets[i];
        while (entry)
        {
            next = entry->next;
            /* signature points into sig_pool, don't free it */
            free(entry->word_indices);
            free(entry);
            entry = next;
        }
    }

    free(ht->buckets);
    free(ht);
    GLOBAL.ht = NULL;

    TRACE("<< hashtable_free");
}

void hashtable_insert(HashTable *ht, const char *sig, size_t word_idx)
{
    unsigned long hash;
    HashEntry *entry;
    size_t new_cap;
    size_t *new_idx;

    TRACE(">> hashtable_insert sig=%s idx=%u", sig ? sig : "NULL",
          (unsigned)word_idx);

    if (!ht || !sig)
    {
        TRACE("<< hashtable_insert (invalid args)");
        return;
    }

    ASSERT_MSG(ht->bucket_count > 0, "bucket_count must be > 0");

    hash = hash_fnv1a(sig) % ht->bucket_count;

    /* Check if signature already exists in this bucket */
    entry = ht->buckets[hash];
    while (entry)
    {
        if (strcmp(entry->signature, sig) == 0)
        {
            /* Add word index to existing entry */
            if (entry->word_count >= entry->word_capacity)
            {
                new_cap = entry->word_capacity * 2;
                new_idx =
                    realloc(entry->word_indices, new_cap * sizeof(size_t));
                if (!new_idx)
                {
                    TRACE("<< hashtable_insert (realloc failed)");
                    return;
                }
                entry->word_indices = new_idx;
                entry->word_capacity = new_cap;
            }
            entry->word_indices[entry->word_count++] = word_idx;

            TRACE("<< hashtable_insert (existing)");

            return;
        }
        entry = entry->next;
    }

    /* Create new entry */
    entry = malloc(sizeof(HashEntry));
    if (!entry)
    {
        TRACE("<< hashtable_insert (malloc failed)");
        return;
    }

    entry->signature = (char *)sig; /* Points into dict's sig_pool */
    entry->word_indices = malloc(4 * sizeof(size_t));
    entry->word_count = 1;
    entry->word_capacity = 4;
    entry->word_indices[0] = word_idx;
    entry->next = ht->buckets[hash];
    ht->buckets[hash] = entry;
    ht->entry_count++;

    TRACE("<< hashtable_insert (new)");
}

HashEntry *hashtable_find(HashTable *ht, const char *sig)
{
    unsigned long hash;
    HashEntry *entry;

    if (!ht || !sig)
    {
        return NULL;
    }

    hash = hash_fnv1a(sig) % ht->bucket_count;
    entry = ht->buckets[hash];

    while (entry)
    {
        if (strcmp(entry->signature, sig) == 0)
        {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}

ChainResults *chain_results_create(void)
{
    ChainResults *res = NULL;
    int success = 0;

    TRACE(">> chain_results_create");

    do
    {
        res = malloc(sizeof(ChainResults));
        if (!res)
        {
            break;
        }

        res->chains = malloc(16 * sizeof(Chain));
        if (!res->chains)
        {
            break;
        }

        res->count = 0;
        res->capacity = 16;
        res->max_length = 0;
        GLOBAL.results = res;

        success = 1;
    } while (0);

    if (!success)
    {
        if (res)
        {
            free(res);
        }

        TRACE("<< chain_results_create (failed)");

        return NULL;
    }

    TRACE("<< chain_results_create");

    return res;
}

void chain_results_add(ChainResults *results, size_t *path, size_t len)
{
    /* Not used - DFS handles chain storage directly */
    UNUSED(results);
    UNUSED(path);
    UNUSED(len);
}

void chain_results_free(ChainResults *results)
{
    size_t i;

    TRACE(">> chain_results_free");

    if (!results)
    {
        TRACE("<< chain_results_free (NULL)");
        return;
    }

    for (i = 0; i < results->count; i++)
    {
        free(results->chains[i].indices);
    }
    free(results->chains);
    free(results);
    GLOBAL.results = NULL;

    TRACE("<< chain_results_free");
}

ChainResults *find_longest_chains(HashTable *ht, Dictionary *dict,
                                  const char *start)
{
    int idx;
    ChainResults *res = NULL;

    TRACE(">> find_longest_chains start=%s", start ? start : "NULL");

    if (!ht || !dict || !start)
    {
        TRACE("<< find_longest_chains (invalid args)");
        return NULL;
    }

    idx = find_word_index(dict, start);
    if (idx < 0)
    {
        TRACE("<< find_longest_chains (word not found)");
        return NULL;
    }

    do
    {
        res = chain_results_create();
        if (!res)
        {
            break;
        }

        /* Allocate path buffer - sized to max chain depth, not dict size */
        GLOBAL.dfs_path = malloc(MAX_CHAIN_DEPTH * sizeof(size_t));
        if (!GLOBAL.dfs_path)
        {
            chain_results_free(res);
            res = NULL;
            break;
        }

        GLOBAL.dfs_path[0] = (size_t)idx;

        TRACE("   starting DFS from idx=%d", idx);

        dfs_dynamic(ht, dict, (size_t)idx, 1);

        free(GLOBAL.dfs_path);
        GLOBAL.dfs_path = NULL;
    } while (0);

    TRACE("<< find_longest_chains chains=%u max_len=%u",
          res ? (unsigned)res->count : 0, res ? (unsigned)res->max_length : 0);

    return res;
}

char *compute_signature(const char *word)
{
    size_t len;

    TRACE(">> compute_signature word=%s", word ? word : "NULL");

    if (!word)
    {
        TRACE("<< compute_signature (NULL)");
        return NULL;
    }

    len = strlen(word);
    if (len >= sizeof(GLOBAL.temp_sig))
    {
        TRACE("<< compute_signature (too long)");
        return NULL;
    }

    memcpy(GLOBAL.temp_sig, word, len + 1);
    sort_chars(GLOBAL.temp_sig, len);

    TRACE("<< compute_signature result=%s", GLOBAL.temp_sig);

    return GLOBAL.temp_sig;
}

int find_word_index(Dictionary *dict, const char *word)
{
    size_t i;

    TRACE(">> find_word_index word=%s", word ? word : "NULL");

    if (!word)
    {
        TRACE("<< find_word_index result=-1 (NULL)");
        return -1;
    }

    if (!dict)
    {
        dict = GLOBAL.dict;
    }

    if (dict)
    {
        for (i = 0; i < dict->count; i++)
        {
            if (strcmp(dict->words[i], word) == 0)
            {
                TRACE("<< find_word_index result=%u", (unsigned)i);
                return (int)i;
            }
        }
    }

    TRACE("<< find_word_index result=-1 (not found)");

    return -1;
}

HashTable *build_index(Dictionary *dict)
{
    HashTable *ht;
    size_t i;

    TRACE(">> build_index");

    if (!dict)
    {
        TRACE("<< build_index (NULL dict)");
        return NULL;
    }

    ht = hashtable_create(dict->count > 1000 ? dict->count : 1000);
    if (!ht)
    {
        TRACE("<< build_index (create failed)");
        return NULL;
    }

    for (i = 0; i < dict->count; i++)
    {
        hashtable_insert(ht, dict->signatures[i], i);
    }

    TRACE("<< build_index entries=%u", (unsigned)ht->entry_count);

    return ht;
}

/* Word accessor for internal.c print functions */
const char *get_word(size_t idx)
{
    if (GLOBAL.dict && idx < GLOBAL.dict->count)
    {
        return GLOBAL.dict->words[idx];
    }
    return "";
}

#else /* Static memory mode */

typedef struct
{
    char signature[POOL_MAX_WORD_LEN];
    size_t word_indices[POOL_MAX_INDICES_PER_ENTRY];
    size_t word_count;
    int next_idx; /* -1 = end of chain */
    int used;
} StaticHashEntry;

typedef struct
{
    /* Word and signature storage - fixed-size 2D arrays */
    char words[POOL_MAX_WORDS][POOL_MAX_WORD_LEN];
    char signatures[POOL_MAX_WORDS][POOL_MAX_WORD_LEN];
    size_t word_count;

    /* Pointer arrays for API compatibility with dynamic mode */
    char *word_ptrs[POOL_MAX_WORDS];
    char *sig_ptrs[POOL_MAX_WORDS];

    /* Dictionary structure */
    Dictionary dictionary;
    int dict_initialized;

    /* Hash table */
    StaticHashEntry hash_entries[POOL_MAX_HASH_ENTRIES];
    int hash_buckets[POOL_HASH_BUCKETS];
    size_t hash_entry_count;

    /* Wrappers to provide API-compatible HashEntry pointers */
    HashEntry hash_entry_wrappers[POOL_MAX_HASH_ENTRIES];

    HashTable hashtable;
    int ht_initialized;

    /* Chain storage */
    size_t chain_indices[POOL_MAX_CHAINS][POOL_MAX_CHAIN_LEN];
    size_t chain_lengths[POOL_MAX_CHAINS];
    Chain chains[POOL_MAX_CHAINS];
    ChainResults chain_results;
    int results_initialized;

    /* DFS working buffers */
    size_t dfs_path[MAX_CHAIN_DEPTH];
    char dfs_candidate[POOL_MAX_WORD_LEN];

    /* Rotating signature buffers for compute_signature */
    char temp_signatures[SIG_BUFFER_COUNT][POOL_MAX_WORD_LEN];
    int temp_sig_idx;
} GlobalState;

static GlobalState G = {0};
#define GLOBAL G

/* Reset all static pools - only reset control variables, not data arrays */
static void reset_all(void)
{
    GLOBAL.word_count = 0;
    GLOBAL.dict_initialized = 0;
    GLOBAL.ht_initialized = 0;
    GLOBAL.results_initialized = 0;
    GLOBAL.hash_entry_count = 0;
    GLOBAL.temp_sig_idx = 0;

    memset(GLOBAL.hash_buckets, 0xFF, sizeof(GLOBAL.hash_buckets));
}

/* DFS search - no visited array, chains always increase in length */
static void dfs_static(size_t cur, size_t depth)
{
    const char *sig;
    size_t sig_len;
    int found;
    int c;
    unsigned long h;
    int idx;
    StaticHashEntry *e;
    size_t i;
    size_t next;
    size_t chain_idx;

    ASSERT_MSG(cur < POOL_MAX_WORDS, "cur index out of bounds");
    ASSERT_MSG(depth <= MAX_CHAIN_DEPTH, "depth exceeds MAX_CHAIN_DEPTH");

    sig = GLOBAL.signatures[cur];
    sig_len = strlen(sig);
    found = 0;

    if (depth >= MAX_CHAIN_DEPTH)
    {
        return;
    }

    for (c = ASCII_MIN; c <= ASCII_MAX; c++)
    {
        insert_sorted(GLOBAL.dfs_candidate, sig, sig_len, (char)c);

        h = hash_fnv1a(GLOBAL.dfs_candidate) % POOL_HASH_BUCKETS;
        idx = GLOBAL.hash_buckets[h];

        while (idx >= 0)
        {
            e = &GLOBAL.hash_entries[idx];
            if (strcmp(e->signature, GLOBAL.dfs_candidate) == 0)
            {
                for (i = 0; i < e->word_count; i++)
                {
                    next = e->word_indices[i];
                    found = 1;
                    GLOBAL.dfs_path[depth] = next;
                    dfs_static(next, depth + 1);
                }
                break;
            }
            idx = e->next_idx;
        }
    }

    if (!found)
    {
        if (depth > GLOBAL.chain_results.max_length)
        {
            GLOBAL.chain_results.count = 0;
            GLOBAL.chain_results.max_length = depth;
        }

        if (depth == GLOBAL.chain_results.max_length &&
            GLOBAL.chain_results.count < POOL_MAX_CHAINS)
        {
            chain_idx = GLOBAL.chain_results.count;
            for (i = 0; i < depth && i < POOL_MAX_CHAIN_LEN; i++)
            {
                GLOBAL.chain_indices[chain_idx][i] = GLOBAL.dfs_path[i];
            }
            GLOBAL.chain_lengths[chain_idx] = depth;
            GLOBAL.chains[chain_idx].indices = GLOBAL.chain_indices[chain_idx];
            GLOBAL.chains[chain_idx].length = depth;
            GLOBAL.chain_results.count++;
        }
    }
}

Dictionary *dictionary_create(size_t cap)
{
    size_t i;

    TRACE(">> dictionary_create cap=%u", (unsigned)cap);

    UNUSED(cap); /* Fixed size in static mode */

    if (!GLOBAL.dict_initialized)
    {
        reset_all();
        for (i = 0; i < POOL_MAX_WORDS; i++)
        {
            GLOBAL.word_ptrs[i] = GLOBAL.words[i];
            GLOBAL.sig_ptrs[i] = GLOBAL.signatures[i];
        }
        GLOBAL.dictionary.words = GLOBAL.word_ptrs;
        GLOBAL.dictionary.signatures = GLOBAL.sig_ptrs;
        GLOBAL.dictionary.count = 0;
        GLOBAL.dictionary.capacity = POOL_MAX_WORDS;
        GLOBAL.dict_initialized = 1;
    }

    TRACE("<< dictionary_create");

    return &GLOBAL.dictionary;
}

void dictionary_free(Dictionary *dict)
{
    TRACE(">> dictionary_free");

    UNUSED(dict);

    GLOBAL.word_count = 0;
    GLOBAL.dictionary.count = 0;
    GLOBAL.dict_initialized = 0;

    TRACE("<< dictionary_free");
}

int dictionary_add(Dictionary *dict, const char *word)
{
    size_t len;

    TRACE(">> dictionary_add word=%s", word ? word : "NULL");

    UNUSED(dict);

    if (!word || GLOBAL.word_count >= POOL_MAX_WORDS)
    {
        TRACE_ERR("invalid or pool full");
        return -1;
    }

    len = strlen(word);
    if (len >= POOL_MAX_WORD_LEN - 1)
    {
        TRACE_ERR("word too long");
        return -1;
    }

    memcpy(GLOBAL.words[GLOBAL.word_count], word, len + 1);
    memcpy(GLOBAL.signatures[GLOBAL.word_count], word, len + 1);
    sort_chars(GLOBAL.signatures[GLOBAL.word_count], len);

    GLOBAL.word_count++;
    GLOBAL.dictionary.count = GLOBAL.word_count;

    TRACE("<< dictionary_add count=%u", (unsigned)GLOBAL.word_count);

    return 0;
}

HashTable *hashtable_create(size_t bucket_count)
{
    TRACE(">> hashtable_create");

    UNUSED(bucket_count);

    memset(GLOBAL.hash_buckets, 0xFF, sizeof(GLOBAL.hash_buckets));

    GLOBAL.hash_entry_count = 0;
    GLOBAL.hashtable.buckets = NULL;
    GLOBAL.hashtable.bucket_count = POOL_HASH_BUCKETS;
    GLOBAL.hashtable.entry_count = 0;
    GLOBAL.ht_initialized = 1;

    TRACE("<< hashtable_create");

    return &GLOBAL.hashtable;
}

void hashtable_free(HashTable *ht)
{
    TRACE(">> hashtable_free");

    UNUSED(ht);

    GLOBAL.hash_entry_count = 0;
    GLOBAL.hashtable.entry_count = 0;
    GLOBAL.ht_initialized = 0;

    TRACE("<< hashtable_free");
}

void hashtable_insert(HashTable *ht, const char *sig, size_t word_idx)
{
    unsigned long h;
    int idx;
    StaticHashEntry *e;
    size_t new_idx;
    size_t sig_len;

    UNUSED(ht);

    TRACE(">> hashtable_insert sig=%s idx=%u", sig ? sig : "NULL",
          (unsigned)word_idx);

    if (!sig)
    {
        TRACE("<< hashtable_insert (NULL sig)");
        return;
    }

    h = hash_fnv1a(sig) % POOL_HASH_BUCKETS;

    idx = GLOBAL.hash_buckets[h];
    while (idx >= 0)
    {
        e = &GLOBAL.hash_entries[idx];
        if (strcmp(e->signature, sig) == 0)
        {
            if (e->word_count < POOL_MAX_INDICES_PER_ENTRY)
            {
                e->word_indices[e->word_count++] = word_idx;
            }

            TRACE("<< hashtable_insert (existing)");
            return;
        }
        idx = e->next_idx;
    }

    if (GLOBAL.hash_entry_count >= POOL_MAX_HASH_ENTRIES)
    {
        TRACE("<< hashtable_insert (pool full)");
        return;
    }

    new_idx = GLOBAL.hash_entry_count++;
    e = &GLOBAL.hash_entries[new_idx];
    sig_len = strlen(sig);

    if (sig_len < POOL_MAX_WORD_LEN)
    {
        memcpy(e->signature, sig, sig_len + 1);
    }

    e->word_indices[0] = word_idx;
    e->word_count = 1;
    e->next_idx = GLOBAL.hash_buckets[h];
    e->used = 1;
    GLOBAL.hash_buckets[h] = (int)new_idx;
    GLOBAL.hashtable.entry_count = GLOBAL.hash_entry_count;

    TRACE("<< hashtable_insert (new)");
}

HashEntry *hashtable_find(HashTable *ht, const char *sig)
{
    unsigned long h;
    int idx;
    StaticHashEntry *se;
    HashEntry *e;

    UNUSED(ht);

    if (!sig)
    {
        return NULL;
    }

    h = hash_fnv1a(sig) % POOL_HASH_BUCKETS;
    idx = GLOBAL.hash_buckets[h];

    while (idx >= 0)
    {
        se = &GLOBAL.hash_entries[idx];
        if (strcmp(se->signature, sig) == 0)
        {
            /* Return wrapper with pointers to static data */
            e = &GLOBAL.hash_entry_wrappers[idx];
            e->signature = se->signature;
            e->word_indices = se->word_indices;
            e->word_count = se->word_count;
            e->word_capacity = POOL_MAX_INDICES_PER_ENTRY;
            e->next = NULL;
            return e;
        }
        idx = se->next_idx;
    }

    return NULL;
}

ChainResults *chain_results_create(void)
{
    TRACE(">> chain_results_create");

    if (!GLOBAL.results_initialized)
    {
        GLOBAL.chain_results.chains = GLOBAL.chains;
        GLOBAL.chain_results.count = 0;
        GLOBAL.chain_results.capacity = POOL_MAX_CHAINS;
        GLOBAL.chain_results.max_length = 0;
        GLOBAL.results_initialized = 1;
    }

    TRACE("<< chain_results_create");

    return &GLOBAL.chain_results;
}

void chain_results_add(ChainResults *results, size_t *path, size_t len)
{
    UNUSED(results);
    UNUSED(path);
    UNUSED(len);
    /* Not used - DFS writes directly to static storage */
}

void chain_results_free(ChainResults *results)
{
    TRACE(">> chain_results_free");

    UNUSED(results);

    GLOBAL.chain_results.count = 0;
    GLOBAL.chain_results.max_length = 0;
    GLOBAL.results_initialized = 0;

    TRACE("<< chain_results_free");
}

ChainResults *find_longest_chains(HashTable *ht, Dictionary *dict,
                                  const char *start)
{
    int idx;
    ChainResults *res;

    TRACE(">> find_longest_chains start=%s", start ? start : "NULL");

    UNUSED(ht);
    UNUSED(dict);

    if (!start)
    {
        TRACE("<< find_longest_chains (NULL start)");
        return NULL;
    }

    idx = find_word_index(NULL, start);
    if (idx < 0)
    {
        TRACE("<< find_longest_chains (word not found)");
        return NULL;
    }

    res = chain_results_create();
    res->count = 0;
    res->max_length = 0;

    GLOBAL.dfs_path[0] = (size_t)idx;

    TRACE("   starting DFS from idx=%d", idx);

    dfs_static((size_t)idx, 1);

    TRACE("<< find_longest_chains chains=%u max_len=%u", (unsigned)res->count,
          (unsigned)res->max_length);

    return res;
}

char *compute_signature(const char *word)
{
    size_t len;
    char *buf;

    TRACE(">> compute_signature word=%s", word ? word : "NULL");

    if (!word)
    {
        TRACE("<< compute_signature (NULL)");
        return NULL;
    }

    len = strlen(word);
    if (len >= POOL_MAX_WORD_LEN)
    {
        TRACE("<< compute_signature (too long)");
        return NULL;
    }

    buf = GLOBAL.temp_signatures[GLOBAL.temp_sig_idx];
    GLOBAL.temp_sig_idx = (GLOBAL.temp_sig_idx + 1) % SIG_BUFFER_COUNT;
    memcpy(buf, word, len + 1);
    sort_chars(buf, len);

    TRACE("<< compute_signature result=%s", buf);

    return buf;
}

int find_word_index(Dictionary *dict, const char *word)
{
    size_t i;

    TRACE(">> find_word_index word=%s", word ? word : "NULL");

    UNUSED(dict);

    if (!word)
    {
        TRACE("<< find_word_index result=-1 (NULL)");
        return -1;
    }

    for (i = 0; i < GLOBAL.word_count; i++)
    {
        if (strcmp(GLOBAL.words[i], word) == 0)
        {
            TRACE("<< find_word_index result=%u", (unsigned)i);
            return (int)i;
        }
    }

    TRACE("<< find_word_index result=-1 (not found)");

    return -1;
}

HashTable *build_index(Dictionary *dict)
{
    HashTable *ht;
    size_t i;

    TRACE(">> build_index");

    UNUSED(dict);

    ht = hashtable_create(POOL_HASH_BUCKETS);

    for (i = 0; i < GLOBAL.word_count; i++)
    {
        hashtable_insert(ht, GLOBAL.signatures[i], i);
    }

    TRACE("<< build_index entries=%u", (unsigned)GLOBAL.hash_entry_count);

    return ht;
}

const char *get_word(size_t idx)
{
    if (idx < GLOBAL.word_count)
    {
        return GLOBAL.words[idx];
    }
    return "";
}

#endif /* USE_DYNAMIC_MEMORY */
