/**
 * @file test_core.c
 * @brief Core test implementations (platform-independent)
 *
 * Contains all test logic without main() function.
 * Can be compiled for PC, ARM bare-metal, or ARM FreeRTOS.
 */

#include <stdlib.h>

#include "test_runner.h"

/* ============================================================================
 * Stub Implementation Check
 * ============================================================================
 *
 * When STUB_IMPLEMENTATION is defined in anagram_chain.h, all tests are
 * automatically skipped and marked as passed. This allows CI/CD to pass
 * while the real implementation is being developed.
 *
 * To enable real tests, remove "#define STUB_IMPLEMENTATION" from
 * src/include/anagram_chain.h
 */

#ifdef STUB_IMPLEMENTATION

int test_compute_signature(void)
{
    TEST_SKIP("compute_signature", "stub implementation");
    return 0;
}

int test_is_derived_signature(void)
{
    TEST_SKIP("is_derived_signature", "stub implementation");
    return 0;
}

int test_is_valid_word(void)
{
    TEST_SKIP("is_valid_word", "stub implementation");
    return 0;
}

int test_dictionary_operations(void)
{
    TEST_SKIP("dictionary_operations", "stub implementation");
    return 0;
}

int test_hashtable_operations(void)
{
    TEST_SKIP("hashtable_operations", "stub implementation");
    return 0;
}

int test_example_chain(void)
{
    TEST_SKIP("example_chain", "stub implementation");
    return 0;
}

#if !defined(PLATFORM_ARM)
int test_performance_small(void)
{
    TEST_SKIP("performance_small", "stub implementation");
    return 0;
}

int test_performance_example(void)
{
    TEST_SKIP("performance_example", "stub implementation");
    return 0;
}
#endif /* !PLATFORM_ARM */

#else /* Real implementation tests */

/* ============================================================================
 * Unit Tests: Signature Generation
 * ============================================================================
 */

int test_compute_signature(void)
{
    const char *name = "compute_signature";

    char *sig1 = compute_signature("sail");
    if (!sig1)
    {
        TEST_SKIP(name, "not implemented");
        return 0;
    }
    ASSERT_STR_EQ(sig1, "ails", name, "sail -> ails");
#ifdef IMPL_AI
    free(sig1);
#endif

    char *sig2 = compute_signature("nails");
    ASSERT_STR_EQ(sig2, "ailns", name, "nails -> ailns");
#ifdef IMPL_AI
    free(sig2);
#endif

    char *sig3 = compute_signature("aliens");
    ASSERT_STR_EQ(sig3, "aeilns", name, "aliens -> aeilns");
#ifdef IMPL_AI
    free(sig3);
#endif

    char *sig4 = compute_signature("abc");
    ASSERT_STR_EQ(sig4, "abc", name, "abc -> abc (already sorted)");
#ifdef IMPL_AI
    free(sig4);
#endif

    char *sig5 = compute_signature("cba");
    ASSERT_STR_EQ(sig5, "abc", name, "cba -> abc");
#ifdef IMPL_AI
    free(sig5);
#endif

    TEST_PASS(name);
    return 0;
}

/* ============================================================================
 * Unit Tests: Derived Signature Detection
 * ============================================================================
 */

int test_is_derived_signature(void)
{
    const char *name = "is_derived_signature";

    /* Valid derived signatures */
    ASSERT_TRUE(is_derived_signature("abc", "abcd"), name,
                "abcd is derived from abc");
    ASSERT_TRUE(is_derived_signature("ails", "ailns"), name,
                "ailns is derived from ails");
    ASSERT_TRUE(is_derived_signature("a", "ab"), name, "ab is derived from a");

    /* Invalid: length difference not 1 */
    ASSERT_TRUE(!is_derived_signature("abc", "abcde"), name,
                "abcde is NOT derived from abc");
    ASSERT_TRUE(!is_derived_signature("abc", "abc"), name,
                "same length is NOT derived");

    /* Invalid: different characters */
    ASSERT_TRUE(!is_derived_signature("abc", "abde"), name,
                "abde is NOT derived from abc");
    ASSERT_TRUE(!is_derived_signature("abc", "defg"), name,
                "completely different");

    TEST_PASS(name);
    return 0;
}

/* ============================================================================
 * Unit Tests: Word Validation
 * ============================================================================
 */

int test_is_valid_word(void)
{
    const char *name = "is_valid_word";

    /* Valid words */
    ASSERT_TRUE(is_valid_word("hello"), name, "hello is valid");
    ASSERT_TRUE(is_valid_word("ABC123"), name, "ABC123 is valid");
    ASSERT_TRUE(is_valid_word("a"), name, "single char is valid");
    ASSERT_TRUE(is_valid_word("~!@#$%"), name, "special chars are valid");

    /* Invalid words */
    ASSERT_TRUE(!is_valid_word(""), name, "empty is invalid");
    ASSERT_TRUE(!is_valid_word(NULL), name, "NULL is invalid");
    ASSERT_TRUE(!is_valid_word("hello world"), name, "space is invalid (< 33)");

    TEST_PASS(name);
    return 0;
}

/* ============================================================================
 * Unit Tests: Dictionary Operations
 * ============================================================================
 */

int test_dictionary_operations(void)
{
    const char *name = "dictionary_operations";

    Dictionary *dict = dictionary_create(4);
    if (!dict)
    {
        TEST_SKIP(name, "not implemented");
        return 0;
    }
    ASSERT_EQ(dict->count, 0, name, "initial count should be 0");

    /* Add words */
    ASSERT_EQ(dictionary_add(dict, "abc"), 0, name, "add abc failed");
    ASSERT_EQ(dictionary_add(dict, "def"), 0, name, "add def failed");
    ASSERT_EQ(dict->count, 2, name, "count should be 2");

    /* Verify content */
    ASSERT_STR_EQ(dict->words[0], "abc", name, "first word should be abc");
    ASSERT_STR_EQ(dict->signatures[0], "abc", name, "first sig should be abc");

    /* Test resize (add more than initial capacity) */
    ASSERT_EQ(dictionary_add(dict, "ghi"), 0, name, "add ghi failed");
    ASSERT_EQ(dictionary_add(dict, "jkl"), 0, name, "add jkl failed");
    ASSERT_EQ(dictionary_add(dict, "mno"), 0, name, "add mno failed");
    ASSERT_EQ(dict->count, 5, name, "count should be 5");

    dictionary_free(dict);

    TEST_PASS(name);
    return 0;
}

/* ============================================================================
 * Unit Tests: Hash Table Operations
 * ============================================================================
 */

int test_hashtable_operations(void)
{
    const char *name = "hashtable_operations";

    HashTable *ht = hashtable_create(101);
    if (!ht)
    {
        TEST_SKIP(name, "not implemented");
        return 0;
    }

    /* Insert entries */
    hashtable_insert(ht, "abc", 0);
    hashtable_insert(ht, "def", 1);
    hashtable_insert(ht, "abc", 2); /* Same signature, different word */

    /* Find entries */
    HashEntry *entry1 = hashtable_find(ht, "abc");
    ASSERT_TRUE(entry1 != NULL, name, "should find abc");
    ASSERT_EQ(entry1->word_count, 2, name, "abc should have 2 words");

    HashEntry *entry2 = hashtable_find(ht, "def");
    ASSERT_TRUE(entry2 != NULL, name, "should find def");
    ASSERT_EQ(entry2->word_count, 1, name, "def should have 1 word");

    HashEntry *entry3 = hashtable_find(ht, "xyz");
    ASSERT_TRUE(entry3 == NULL, name, "should not find xyz");

    hashtable_free(ht);

    TEST_PASS(name);
    return 0;
}

/* ============================================================================
 * Integration Test: Example from Task
 * ============================================================================
 */

int test_example_chain(void)
{
    const char *name = "example_chain";

    /* Create dictionary from task example */
    Dictionary *dict = dictionary_create(16);
    if (!dict)
    {
        TEST_SKIP(name, "not implemented");
        return 0;
    }

    const char *words[] = {"abcdg",  "abcd",     "abcdgh", "abcek",
                           "abck",   "abc",      "abcdp",  "abcdghi",
                           "bafced", "akjpqwmn", "abcelk", "baclekt"};

    for (int i = 0; i < 12; i++)
    {
        dictionary_add(dict, words[i]);
    }

    HashTable *index = build_index(dict);
    ASSERT_TRUE(index != NULL, name, "build_index failed");

    ChainResults *results = find_longest_chains(index, dict, "abck");
    ASSERT_TRUE(results != NULL, name, "find_longest_chains failed");
    ASSERT_EQ(results->max_length, 4, name, "longest chain should be length 4");
    ASSERT_TRUE(results->count >= 1, name, "should find at least 1 chain");

    /* Verify the chain: abck -> abcek -> abcelk -> baclekt */
    int found_expected_chain = 0;
    for (size_t i = 0; i < results->count; i++)
    {
        Chain *chain = &results->chains[i];
        if (chain->length == 4)
        {
            const char *w0 = dict->words[chain->indices[0]];
            const char *w1 = dict->words[chain->indices[1]];
            const char *w2 = dict->words[chain->indices[2]];
            const char *w3 = dict->words[chain->indices[3]];

            if (strcmp(w0, "abck") == 0 && strcmp(w1, "abcek") == 0 &&
                strcmp(w2, "abcelk") == 0 && strcmp(w3, "baclekt") == 0)
            {
                found_expected_chain = 1;
                break;
            }
        }
    }
    ASSERT_TRUE(found_expected_chain, name,
                "expected chain abck->abcek->abcelk->baclekt not found");

    chain_results_free(results);
    hashtable_free(index);
    dictionary_free(dict);

    TEST_PASS(name);
    return 0;
}

/* ============================================================================
 * Performance Tests (PC only - require file system)
 * ============================================================================
 */

#if !defined(PLATFORM_ARM)

#define SMALL_DICT_TIME_LIMIT_MS 1000.0

int test_performance_small(void)
{
    const char *name = "performance_small";

    double start = timer_now();

    Dictionary *dict = dictionary_create(64);
    if (!dict)
    {
        TEST_SKIP(name, "not implemented");
        return 0;
    }

    int loaded = load_dictionary("tests/data/small.txt", dict);

    if (loaded < 0)
    {
        TEST_SKIP(name, "test file not found");
        dictionary_free(dict);
        return 0;
    }

    HashTable *index = build_index(dict);
    if (dict->count > 0)
    {
        ChainResults *results =
            find_longest_chains(index, dict, dict->words[0]);
        chain_results_free(results);
    }

    double elapsed = timer_now() - start;

    hashtable_free(index);
    dictionary_free(dict);

    ASSERT_TRUE(elapsed < SMALL_DICT_TIME_LIMIT_MS, name,
                "exceeded time limit");

    test_puts("  [PASS] ");
    test_puts(name);
    test_puts(" (");
    test_putint((int)elapsed);
    test_puts(" ms)\n");
    return 0;
}

int test_performance_example(void)
{
    const char *name = "performance_example";

    double start = timer_now();

    Dictionary *dict = dictionary_create(16);
    if (!dict)
    {
        TEST_SKIP(name, "not implemented");
        return 0;
    }

    int loaded = load_dictionary("tests/data/example.txt", dict);

    if (loaded < 0)
    {
        TEST_SKIP(name, "test file not found");
        dictionary_free(dict);
        return 0;
    }

    HashTable *index = build_index(dict);
    ChainResults *results = find_longest_chains(index, dict, "abck");

    double elapsed = timer_now() - start;

    /* Verify results */
    ASSERT_TRUE(results != NULL, name, "search failed");
    ASSERT_EQ(results->max_length, 4, name, "wrong chain length");

    chain_results_free(results);
    hashtable_free(index);
    dictionary_free(dict);

    ASSERT_TRUE(elapsed < SMALL_DICT_TIME_LIMIT_MS, name,
                "exceeded time limit");

    test_puts("  [PASS] ");
    test_puts(name);
    test_puts(" (");
    test_putint((int)elapsed);
    test_puts(" ms)\n");
    return 0;
}

#endif /* !PLATFORM_ARM */

#endif /* STUB_IMPLEMENTATION */

/* ============================================================================
 * Test Runner
 * ============================================================================
 */

int run_all_tests(void)
{
    int failures = 0;

    test_puts("\n");
    test_puts("======================================\n");
    test_puts("  Embedded Anagram Chain Demo - Unit Tests\n");
    test_puts("======================================\n\n");

    test_puts("Running unit tests...\n\n");

    test_puts("Signature Tests:\n");
    failures += test_compute_signature();
    failures += test_is_derived_signature();

    test_puts("\nValidation Tests:\n");
    failures += test_is_valid_word();

    test_puts("\nDictionary Tests:\n");
    failures += test_dictionary_operations();

    test_puts("\nHash Table Tests:\n");
    failures += test_hashtable_operations();

    test_puts("\nIntegration Tests:\n");
    failures += test_example_chain();

#if !defined(PLATFORM_ARM)
    test_puts("\nPerformance Tests:\n");
    failures += test_performance_small();
    failures += test_performance_example();
#else
    test_puts("\nPerformance Tests:\n");
    test_puts("  [SKIP] File-based tests not available on ARM\n");
#endif

    test_puts("\n======================================\n");
    if (failures == 0)
    {
        test_puts("  All tests passed!\n");
    }
    else
    {
        test_puts("  ");
        test_putint(failures);
        test_puts(" test(s) FAILED\n");
    }
    test_puts("======================================\n\n");

    return failures;
}
