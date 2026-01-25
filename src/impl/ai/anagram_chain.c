/**
 * @file anagram_chain.c
 * @brief Implementation of Embedded Anagram Chain Demo
 *
 * A derived anagram is a word consisting of all letters from the base word
 * plus one additional letter (rearranged). Example: sail -> nails -> aliens
 *
 * Algorithm:
 * 1. Load dictionary and compute signature (sorted chars) for each word
 * 2. Build hash table mapping signatures to word indices
 * 3. Use DFS to find all longest chains from starting word
 *
 * Time complexity: O(n * m * log(m)) for indexing, O(chains * depth) for search
 * Space complexity: O(n * m) where n = word count, m = avg word length
 */

#include "anagram_chain.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ============================================================================
 * Internal Helper Functions
 * ============================================================================
 */

/**
 * @brief Duplicate a string (portable strdup)
 * @param s Source string
 * @return Newly allocated copy, or NULL on failure
 */
static char *str_dup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}

/**
 * @brief Comparison function for qsort (sorts characters)
 */
static int compare_chars(const void *a, const void *b) {
    return (*(const char *)a) - (*(const char *)b);
}

/**
 * @brief djb2 hash function for strings
 * @param str Input string
 * @return Hash value
 */
static unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/**
 * @brief DFS search for anagram chains
 * @param ht Hash table index
 * @param dict Dictionary
 * @param current_idx Current word index
 * @param path Current path (array of indices)
 * @param path_len Current path length
 * @param visited Visited flags array
 * @param results Results accumulator
 */
static void dfs_search(HashTable *ht, Dictionary *dict, size_t current_idx,
                       size_t *path, size_t path_len, int *visited,
                       ChainResults *results) {
    const char *current_sig = dict->signatures[current_idx];
    size_t current_len = strlen(current_sig);
    int found_next = 0;

    /* Try each possible additional character (ASCII 33-126) */
    for (int c = 33; c <= 126; c++) {
        /* Build candidate signature: current signature + new char, sorted */
        char *candidate = malloc(current_len + 2);
        if (!candidate) continue;

        memcpy(candidate, current_sig, current_len);
        candidate[current_len] = (char)c;
        candidate[current_len + 1] = '\0';
        qsort(candidate, current_len + 1, sizeof(char), compare_chars);

        /* Look up in hash table */
        HashEntry *entry = hashtable_find(ht, candidate);
        if (entry) {
            /* Check each word with this signature */
            for (size_t i = 0; i < entry->word_count; i++) {
                size_t next_idx = entry->word_indices[i];
                if (!visited[next_idx]) {
                    found_next = 1;
                    visited[next_idx] = 1;
                    path[path_len] = next_idx;
                    dfs_search(ht, dict, next_idx, path, path_len + 1, visited,
                               results);
                    visited[next_idx] = 0;
                }
            }
        }

        free(candidate);
    }

    /* If no next word found, this is end of chain */
    if (!found_next) {
        chain_results_add(results, path, path_len);
    }
}

/* ============================================================================
 * Timer Functions
 * ============================================================================
 */

double timer_now(void) {
#if defined(PLATFORM_ARM)
    /* ARM platforms don't have clock_gettime, return 0 */
    return 0.0;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
#endif
}

void timer_print(const char *label, double start_ms, double end_ms) {
    double elapsed = end_ms - start_ms;
    if (elapsed >= 1000.0) {
        printf("%s: %.2f s\n", label, elapsed / 1000.0);
    } else {
        printf("%s: %.3f ms\n", label, elapsed);
    }
}

/* ============================================================================
 * Utility Functions
 * ============================================================================
 */

char *compute_signature(const char *word) {
    if (!word) return NULL;
    size_t len = strlen(word);
    char *sig = malloc(len + 1);
    if (!sig) return NULL;

    memcpy(sig, word, len + 1);
    qsort(sig, len, sizeof(char), compare_chars);
    return sig;
}

int is_valid_word(const char *word) {
    if (!word || !*word) return 0;

    size_t len = 0;
    for (const char *p = word; *p; p++) {
        if (*p < 33 || *p > 126) return 0;
        len++;
    }

    return (len >= 1 && len <= 255);
}

int is_derived_signature(const char *sig1, const char *sig2) {
    if (!sig1 || !sig2) return 0;

    size_t len1 = strlen(sig1);
    size_t len2 = strlen(sig2);

    /* Derived signature must be exactly 1 character longer */
    if (len2 != len1 + 1) return 0;

    /* Two-pointer comparison */
    size_t i = 0; /* Pointer for sig1 */
    size_t j = 0; /* Pointer for sig2 */
    int extra_found = 0;

    while (j < len2) {
        if (i < len1 && sig1[i] == sig2[j]) {
            i++;
            j++;
        } else if (!extra_found) {
            extra_found = 1;
            j++;
        } else {
            return 0; /* More than one extra character */
        }
    }

    return (i == len1);
}

/* ============================================================================
 * Dictionary Functions
 * ============================================================================
 */

Dictionary *dictionary_create(size_t initial_capacity) {
    Dictionary *dict = malloc(sizeof(Dictionary));
    if (!dict) return NULL;

    dict->words = malloc(initial_capacity * sizeof(char *));
    dict->signatures = malloc(initial_capacity * sizeof(char *));
    if (!dict->words || !dict->signatures) {
        free(dict->words);
        free(dict->signatures);
        free(dict);
        return NULL;
    }

    dict->count = 0;
    dict->capacity = initial_capacity;
    return dict;
}

void dictionary_free(Dictionary *dict) {
    if (!dict) return;

    for (size_t i = 0; i < dict->count; i++) {
        free(dict->words[i]);
        free(dict->signatures[i]);
    }
    free(dict->words);
    free(dict->signatures);
    free(dict);
}

int dictionary_add(Dictionary *dict, const char *word) {
    if (!dict || !word) return -1;

    /* Resize if needed */
    if (dict->count >= dict->capacity) {
        size_t new_capacity = dict->capacity * 2;
        char **new_words = realloc(dict->words, new_capacity * sizeof(char *));
        char **new_sigs =
            realloc(dict->signatures, new_capacity * sizeof(char *));
        if (!new_words || !new_sigs) {
            return -1;
        }
        dict->words = new_words;
        dict->signatures = new_sigs;
        dict->capacity = new_capacity;
    }

    /* Add word and compute signature */
    dict->words[dict->count] = str_dup(word);
    dict->signatures[dict->count] = compute_signature(word);

    if (!dict->words[dict->count] || !dict->signatures[dict->count]) {
        free(dict->words[dict->count]);
        free(dict->signatures[dict->count]);
        return -1;
    }

    dict->count++;
    return 0;
}

int load_dictionary(const char *filename, Dictionary *dict) {
    if (!filename || !dict) return -1;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return -1;
    }

    char line[MAX_WORD_LENGTH + 2]; /* +2 for newline and null */
    int loaded = 0;

    while (fgets(line, sizeof(line), fp)) {
        /* Strip trailing whitespace */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' ||
                           line[len - 1] == ' ' || line[len - 1] == '\t')) {
            line[--len] = '\0';
        }

        /* Skip empty lines */
        if (len == 0) continue;

        /* Validate and add word */
        if (is_valid_word(line)) {
            if (dictionary_add(dict, line) == 0) {
                loaded++;
            }
        }
    }

    fclose(fp);
    return loaded;
}

int find_word_index(Dictionary *dict, const char *word) {
    if (!dict || !word) return -1;

    for (size_t i = 0; i < dict->count; i++) {
        if (strcmp(dict->words[i], word) == 0) {
            return (int)i;
        }
    }
    return -1;
}

/* ============================================================================
 * Hash Table Functions
 * ============================================================================
 */

HashTable *hashtable_create(size_t bucket_count) {
    HashTable *ht = malloc(sizeof(HashTable));
    if (!ht) return NULL;

    ht->buckets = calloc(bucket_count, sizeof(HashEntry *));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }

    ht->bucket_count = bucket_count;
    ht->entry_count = 0;
    return ht;
}

void hashtable_free(HashTable *ht) {
    if (!ht) return;

    for (size_t i = 0; i < ht->bucket_count; i++) {
        HashEntry *entry = ht->buckets[i];
        while (entry) {
            HashEntry *next = entry->next;
            free(entry->signature);
            free(entry->word_indices);
            free(entry);
            entry = next;
        }
    }

    free(ht->buckets);
    free(ht);
}

void hashtable_insert(HashTable *ht, const char *signature, size_t word_index) {
    if (!ht || !signature) return;

    unsigned long hash = hash_string(signature) % ht->bucket_count;

    /* Search for existing entry with same signature */
    HashEntry *entry = ht->buckets[hash];
    while (entry) {
        if (strcmp(entry->signature, signature) == 0) {
            /* Add to existing entry */
            if (entry->word_count >= entry->word_capacity) {
                size_t new_cap = entry->word_capacity * 2;
                size_t *new_indices =
                    realloc(entry->word_indices, new_cap * sizeof(size_t));
                if (!new_indices) return;
                entry->word_indices = new_indices;
                entry->word_capacity = new_cap;
            }
            entry->word_indices[entry->word_count++] = word_index;
            return;
        }
        entry = entry->next;
    }

    /* Create new entry */
    entry = malloc(sizeof(HashEntry));
    if (!entry) return;

    entry->signature = str_dup(signature);
    entry->word_indices = malloc(4 * sizeof(size_t));
    entry->word_count = 1;
    entry->word_capacity = 4;
    entry->word_indices[0] = word_index;

    /* Insert at head of bucket */
    entry->next = ht->buckets[hash];
    ht->buckets[hash] = entry;
    ht->entry_count++;
}

HashEntry *hashtable_find(HashTable *ht, const char *signature) {
    if (!ht || !signature) return NULL;

    unsigned long hash = hash_string(signature) % ht->bucket_count;

    HashEntry *entry = ht->buckets[hash];
    while (entry) {
        if (strcmp(entry->signature, signature) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

HashTable *build_index(Dictionary *dict) {
    if (!dict) return NULL;

    HashTable *ht = hashtable_create(HASH_TABLE_SIZE);
    if (!ht) return NULL;

    for (size_t i = 0; i < dict->count; i++) {
        hashtable_insert(ht, dict->signatures[i], i);
    }

    return ht;
}

/* ============================================================================
 * Chain Finder Functions
 * ============================================================================
 */

ChainResults *chain_results_create(void) {
    ChainResults *results = malloc(sizeof(ChainResults));
    if (!results) return NULL;

    results->chains = malloc(16 * sizeof(Chain));
    results->count = 0;
    results->capacity = 16;
    results->max_length = 0;

    return results;
}

void chain_results_add(ChainResults *results, size_t *path, size_t path_len) {
    if (!results || !path || path_len == 0) return;

    /* Only keep chains of maximum length */
    if (path_len < results->max_length) return;

    /* If this chain is longer, clear existing shorter chains */
    if (path_len > results->max_length) {
        /* Free existing chains */
        for (size_t i = 0; i < results->count; i++) {
            free(results->chains[i].indices);
        }
        results->count = 0;
        results->max_length = path_len;
    }

    /* Limit number of stored chains */
    if (results->count >= MAX_CHAINS) return;

    /* Resize if needed */
    if (results->count >= results->capacity) {
        size_t new_cap = results->capacity * 2;
        Chain *new_chains = realloc(results->chains, new_cap * sizeof(Chain));
        if (!new_chains) return;
        results->chains = new_chains;
        results->capacity = new_cap;
    }

    /* Copy path */
    size_t *indices = malloc(path_len * sizeof(size_t));
    if (!indices) return;
    memcpy(indices, path, path_len * sizeof(size_t));

    results->chains[results->count].indices = indices;
    results->chains[results->count].length = path_len;
    results->count++;
}

void chain_results_free(ChainResults *results) {
    if (!results) return;

    for (size_t i = 0; i < results->count; i++) {
        free(results->chains[i].indices);
    }
    free(results->chains);
    free(results);
}

ChainResults *find_longest_chains(HashTable *ht, Dictionary *dict,
                                  const char *start_word) {
    if (!ht || !dict || !start_word) return NULL;

    int start_idx = find_word_index(dict, start_word);
    if (start_idx < 0) return NULL;

    ChainResults *results = chain_results_create();
    if (!results) return NULL;

    /* Allocate path and visited arrays */
    size_t *path = malloc(dict->count * sizeof(size_t));
    int *visited = calloc(dict->count, sizeof(int));

    if (!path || !visited) {
        free(path);
        free(visited);
        chain_results_free(results);
        return NULL;
    }

    /* Initialize search from start word */
    path[0] = (size_t)start_idx;
    visited[start_idx] = 1;

    /* Run DFS */
    dfs_search(ht, dict, (size_t)start_idx, path, 1, visited, results);

    free(path);
    free(visited);

    return results;
}

/* ============================================================================
 * Output Functions
 * ============================================================================
 */

void print_chain(Dictionary *dict, Chain *chain) {
    for (size_t i = 0; i < chain->length; i++) {
        printf("%s", dict->words[chain->indices[i]]);
        if (i < chain->length - 1) {
            printf("->");
        }
    }
    printf("\n");
}

void print_results(Dictionary *dict, ChainResults *results) {
    if (!results || results->count == 0) {
        printf("No chains found.\n");
        return;
    }

    printf("\nFound %zu chain(s) of length %zu:\n", results->count,
           results->max_length);
    for (size_t i = 0; i < results->count; i++) {
        print_chain(dict, &results->chains[i]);
    }
}

void print_usage(const char *program_name) {
    printf("Embedded Anagram Chain Demo\n");
    printf("===========================\n\n");
    printf("Finds the longest chain of derived anagrams in a dictionary.\n\n");
    printf("Usage: %s <dictionary_file> <starting_word>\n\n", program_name);
    printf("Arguments:\n");
    printf("  dictionary_file  Path to dictionary file (one word per line)\n");
    printf("  starting_word    Word to start the chain from\n\n");
    printf("Example:\n");
    printf("  %s words.txt abc\n", program_name);
}
