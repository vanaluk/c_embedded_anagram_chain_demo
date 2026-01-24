/**
 * @file test_runner.h
 * @brief Platform-independent test runner interface
 *
 * Provides test macros and output abstraction for running tests on:
 * - PC (stdout)
 * - ARM bare-metal (UART)
 * - ARM FreeRTOS (UART)
 */

#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <string.h>

#include "anagram_chain.h"

/* ============================================================================
 * Platform-specific Output
 * ============================================================================
 */

#if defined(PLATFORM_ARM)
/* ARM platforms use UART */
#include "uart.h"
#define test_puts(s) uart_puts(s)
#define test_putint(n) uart_putint(n)
#else
/* PC uses stdio */
#include <stdio.h>
#define test_puts(s) printf("%s", s)
#define test_putint(n) printf("%d", n)
#endif

/* ============================================================================
 * Test Macros
 * ============================================================================
 */

#define TEST_PASS(name)         \
    do {                        \
        test_puts("  [PASS] "); \
        test_puts(name);        \
        test_puts("\n");        \
    } while (0)

#define TEST_SKIP(name, reason) \
    do {                        \
        test_puts("  [SKIP] "); \
        test_puts(name);        \
        test_puts(": ");        \
        test_puts(reason);      \
        test_puts("\n");        \
    } while (0)

#define TEST_FAIL(name, msg)    \
    do {                        \
        test_puts("  [FAIL] "); \
        test_puts(name);        \
        test_puts(": ");        \
        test_puts(msg);         \
        test_puts("\n");        \
        return 1;               \
    } while (0)

#define ASSERT_TRUE(cond, name, msg)       \
    do {                                   \
        if (!(cond)) TEST_FAIL(name, msg); \
    } while (0)

#define ASSERT_EQ(a, b, name, msg)            \
    do {                                      \
        if ((a) != (b)) TEST_FAIL(name, msg); \
    } while (0)

#define ASSERT_STR_EQ(a, b, name, msg)                   \
    do {                                                 \
        if (strcmp((a), (b)) != 0) TEST_FAIL(name, msg); \
    } while (0)

/* ============================================================================
 * Test Functions (implemented in test_core.c)
 * ============================================================================
 */

int test_compute_signature(void);
int test_is_derived_signature(void);
int test_is_valid_word(void);
int test_dictionary_operations(void);
int test_hashtable_operations(void);
int test_example_chain(void);

/* File-based tests (PC only) */
#if !defined(PLATFORM_ARM)
int test_performance_small(void);
int test_performance_example(void);
#endif

/**
 * @brief Run all tests and return number of failures
 */
int run_all_tests(void);

#endif /* TEST_RUNNER_H */
