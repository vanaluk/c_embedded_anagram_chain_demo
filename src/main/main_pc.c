/**
 * @file main_pc.c
 * @brief PC/Native main entry point for Anagram Chain Finder
 *
 * Command-line interface for running on desktop/server systems.
 */

#include <stdio.h>
#include <stdlib.h>

#include "anagram_chain.h"

int main(int argc, char *argv[]) {
    /* Validate arguments */
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *dict_file = argv[1];
    const char *start_word = argv[2];

    /* Start total timer */
    double total_start = timer_now();

    /* Load dictionary */
    printf("Loading dictionary: %s\n", dict_file);
    double load_start = timer_now();

    Dictionary *dict = dictionary_create(INITIAL_CAPACITY);
    if (!dict) {
        fprintf(stderr, "Error: Failed to create dictionary\n");
        return 1;
    }

    int loaded = load_dictionary(dict_file, dict);
    if (loaded < 0) {
        dictionary_free(dict);
        return 1;
    }

    timer_print("Dictionary loaded", load_start, timer_now());
    printf("Words loaded: %zu\n", dict->count);

    /* Verify start word exists */
    if (find_word_index(dict, start_word) < 0) {
        fprintf(stderr, "Error: Starting word '%s' not found in dictionary\n",
                start_word);
        dictionary_free(dict);
        return 1;
    }

    /* Build index */
    printf("\nBuilding index...\n");
    double index_start = timer_now();
    HashTable *index = build_index(dict);
    if (!index) {
        fprintf(stderr, "Error: Failed to build index\n");
        dictionary_free(dict);
        return 1;
    }
    timer_print("Index built", index_start, timer_now());
    printf("Unique signatures: %zu\n", index->entry_count);

    /* Find chains */
    printf("\nSearching for longest chains starting from '%s'...\n",
           start_word);
    double search_start = timer_now();
    ChainResults *results = find_longest_chains(index, dict, start_word);
    double search_end = timer_now();
    timer_print("Search completed", search_start, search_end);

    /* Print results */
    print_results(dict, results);

    /* Print total time */
    double total_end = timer_now();
    printf("\nTotal execution time: ");
    timer_print("Total", total_start, total_end);

    /* Cleanup */
    chain_results_free(results);
    hashtable_free(index);
    dictionary_free(dict);

    return 0;
}
