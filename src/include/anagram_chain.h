/**
 * @file anagram_chain.h
 * @brief Public interface for Anagram Chain Finder
 *
 * A derived anagram is a word consisting of all letters from the base word
 * plus one additional letter (rearranged). Example: sail -> nails -> aliens
 */

#ifndef ANAGRAM_CHAIN_H
#define ANAGRAM_CHAIN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Constants
 * ============================================================================
 */

#define MAX_WORD_LENGTH 256
#define INITIAL_CAPACITY 1024
#define HASH_TABLE_SIZE 100003
#define MAX_CHAINS 10000

/* ============================================================================
 * Data Structures
 * ============================================================================
 */

/**
 * @brief Dictionary structure holding all words and their signatures
 */
typedef struct {
    char **words;      /**< Array of word strings */
    char **signatures; /**< Parallel array of sorted signatures */
    size_t count;      /**< Number of words */
    size_t capacity;   /**< Allocated capacity */
} Dictionary;

/**
 * @brief Hash table entry for signature-to-words mapping
 */
typedef struct HashEntry {
    char *signature;        /**< Key: sorted character signature */
    size_t *word_indices;   /**< Values: indices into Dictionary.words */
    size_t word_count;      /**< Number of words with this signature */
    size_t word_capacity;   /**< Allocated capacity for word_indices */
    struct HashEntry *next; /**< Next entry in collision chain */
} HashEntry;

/**
 * @brief Hash table for O(1) signature lookup
 */
typedef struct {
    HashEntry **buckets; /**< Array of bucket pointers */
    size_t bucket_count; /**< Number of buckets */
    size_t entry_count;  /**< Total unique signatures */
} HashTable;

/**
 * @brief A single chain of words
 */
typedef struct {
    size_t *indices; /**< Word indices in chain order */
    size_t length;   /**< Chain length */
} Chain;

/**
 * @brief Collection of found chains
 */
typedef struct {
    Chain *chains;     /**< Array of chains */
    size_t count;      /**< Number of chains */
    size_t capacity;   /**< Allocated capacity */
    size_t max_length; /**< Length of longest chains */
} ChainResults;

/* ============================================================================
 * Timer Functions
 * ============================================================================
 */

/**
 * @brief Get current time in milliseconds
 * @return Current time in milliseconds
 */
double timer_now(void);

/**
 * @brief Print elapsed time with label
 * @param label Description label
 * @param start_ms Start time in milliseconds
 * @param end_ms End time in milliseconds
 */
void timer_print(const char *label, double start_ms, double end_ms);

/* ============================================================================
 * Utility Functions
 * ============================================================================
 */

/**
 * @brief Compute signature (sorted characters) of a word
 * @param word Input word
 * @return Newly allocated signature string, caller must free
 */
char *compute_signature(const char *word);

/**
 * @brief Check if a word contains only valid ASCII characters (33-126)
 * @param word Word to validate
 * @return 1 if valid, 0 otherwise
 */
int is_valid_word(const char *word);

/**
 * @brief Check if sig2 is a derived signature of sig1
 * @param sig1 Base signature
 * @param sig2 Candidate derived signature
 * @return 1 if sig2 is derived from sig1, 0 otherwise
 */
int is_derived_signature(const char *sig1, const char *sig2);

/* ============================================================================
 * Dictionary Functions
 * ============================================================================
 */

/**
 * @brief Create a new dictionary
 * @param initial_capacity Initial array capacity
 * @return Newly allocated dictionary, or NULL on failure
 */
Dictionary *dictionary_create(size_t initial_capacity);

/**
 * @brief Free dictionary memory
 * @param dict Dictionary to free
 */
void dictionary_free(Dictionary *dict);

/**
 * @brief Add a word to the dictionary
 * @param dict Dictionary to add to
 * @param word Word to add
 * @return 0 on success, -1 on failure
 */
int dictionary_add(Dictionary *dict, const char *word);

/**
 * @brief Load dictionary from file
 * @param filename Path to dictionary file
 * @param dict Dictionary to populate
 * @return Number of words loaded, or -1 on error
 */
int load_dictionary(const char *filename, Dictionary *dict);

/**
 * @brief Find word index in dictionary
 * @param dict Dictionary to search
 * @param word Word to find
 * @return Index of word, or -1 if not found
 */
int find_word_index(Dictionary *dict, const char *word);

/* ============================================================================
 * Hash Table Functions
 * ============================================================================
 */

/**
 * @brief Create a new hash table
 * @param bucket_count Number of buckets
 * @return Newly allocated hash table, or NULL on failure
 */
HashTable *hashtable_create(size_t bucket_count);

/**
 * @brief Free hash table memory
 * @param ht Hash table to free
 */
void hashtable_free(HashTable *ht);

/**
 * @brief Insert word index with signature into hash table
 * @param ht Hash table
 * @param signature Signature key
 * @param word_index Word index value
 */
void hashtable_insert(HashTable *ht, const char *signature, size_t word_index);

/**
 * @brief Find entry by signature
 * @param ht Hash table
 * @param signature Signature to find
 * @return Hash entry, or NULL if not found
 */
HashEntry *hashtable_find(HashTable *ht, const char *signature);

/**
 * @brief Build hash index from dictionary
 * @param dict Dictionary to index
 * @return Newly allocated hash table
 */
HashTable *build_index(Dictionary *dict);

/* ============================================================================
 * Chain Finder Functions
 * ============================================================================
 */

/**
 * @brief Create chain results structure
 * @return Newly allocated ChainResults
 */
ChainResults *chain_results_create(void);

/**
 * @brief Add a chain to results
 * @param results Results structure
 * @param path Array of word indices
 * @param path_len Length of path
 */
void chain_results_add(ChainResults *results, size_t *path, size_t path_len);

/**
 * @brief Free chain results memory
 * @param results Results to free
 */
void chain_results_free(ChainResults *results);

/**
 * @brief Find all longest chains starting from a word
 * @param ht Hash table index
 * @param dict Dictionary
 * @param start_word Starting word
 * @return Chain results, or NULL on error
 */
ChainResults *find_longest_chains(HashTable *ht, Dictionary *dict,
                                  const char *start_word);

/* ============================================================================
 * Output Functions
 * ============================================================================
 */

/**
 * @brief Print a single chain
 * @param dict Dictionary
 * @param chain Chain to print
 */
void print_chain(Dictionary *dict, Chain *chain);

/**
 * @brief Print all results
 * @param dict Dictionary
 * @param results Chain results
 */
void print_results(Dictionary *dict, ChainResults *results);

/**
 * @brief Print usage message
 * @param program_name Program name (argv[0])
 */
void print_usage(const char *program_name);

#ifdef __cplusplus
}
#endif

#endif /* ANAGRAM_CHAIN_H */
