/**
 * @file test_core.c
 * @brief Core test implementations (platform-independent)
 *
 * Contains all test logic without main() function.
 * Can be compiled for PC, ARM bare-metal, or ARM FreeRTOS.
 *
 * TODO: Implement your own tests
 */

#include "test_runner.h"

/* ============================================================================
 * Unit Tests: Signature Generation
 * ============================================================================
 */

int test_compute_signature(void) {
    const char *name = "compute_signature";

    /* TODO: Implement */
    TEST_SKIP(name, "not implemented");
    return 0;
}

/* ============================================================================
 * Unit Tests: Derived Signature Detection
 * ============================================================================
 */

int test_is_derived_signature(void) {
    const char *name = "is_derived_signature";

    /* TODO: Implement */
    TEST_SKIP(name, "not implemented");
    return 0;
}

/* ============================================================================
 * Unit Tests: Word Validation
 * ============================================================================
 */

int test_is_valid_word(void) {
    const char *name = "is_valid_word";

    /* TODO: Implement */
    TEST_SKIP(name, "not implemented");
    return 0;
}

/* ============================================================================
 * Unit Tests: Dictionary Operations
 * ============================================================================
 */

int test_dictionary_operations(void) {
    const char *name = "dictionary_operations";

    /* TODO: Implement */
    TEST_SKIP(name, "not implemented");
    return 0;
}

/* ============================================================================
 * Unit Tests: Hash Table Operations
 * ============================================================================
 */

int test_hashtable_operations(void) {
    const char *name = "hashtable_operations";

    /* TODO: Implement */
    TEST_SKIP(name, "not implemented");
    return 0;
}

/* ============================================================================
 * Integration Test: Example from Task
 * ============================================================================
 */

int test_example_chain(void) {
    const char *name = "example_chain";

    /* TODO: Implement */
    TEST_SKIP(name, "not implemented");
    return 0;
}

/* ============================================================================
 * Performance Tests (PC only - require file system)
 * ============================================================================
 */

#if !defined(PLATFORM_ARM)

int test_performance_small(void) {
    const char *name = "performance_small";

    /* TODO: Implement */
    TEST_SKIP(name, "not implemented");
    return 0;
}

int test_performance_example(void) {
    const char *name = "performance_example";

    /* TODO: Implement */
    TEST_SKIP(name, "not implemented");
    return 0;
}

#endif /* !PLATFORM_ARM */

/* ============================================================================
 * Test Runner
 * ============================================================================
 */

int run_all_tests(void) {
    int failures = 0;

    test_puts("\n");
    test_puts("======================================\n");
    test_puts("  Anagram Chain Finder - Unit Tests\n");
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
    if (failures == 0) {
        test_puts("  All tests passed!\n");
    } else {
        test_puts("  ");
        test_putint(failures);
        test_puts(" test(s) FAILED\n");
    }
    test_puts("======================================\n\n");

    return failures;
}
