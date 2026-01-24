/* Include the main source file for testing */
#ifndef TEST_BUILD
#define TEST_BUILD
#endif
#include "../src/anagram_chain.c"

#include <assert.h>

/* ============================================================================
 * Test Utilities
 * ============================================================================ */

#define TEST_PASS(name) printf("  [PASS] %s\n", name)
#define TEST_FAIL(name, msg)                                                                       \
    do {                                                                                           \
        printf("  [FAIL] %s: %s\n", name, msg);                                                    \
        return 1;                                                                                  \
    } while (0)

#define ASSERT_TRUE(cond, name, msg)                                                               \
    do {                                                                                           \
        if (!(cond)) TEST_FAIL(name, msg);                                                         \
    } while (0)

#define ASSERT_EQ(a, b, name, msg)                                                                 \
    do {                                                                                           \
        if ((a) != (b)) TEST_FAIL(name, msg);                                                      \
    } while (0)

#define ASSERT_STR_EQ(a, b, name, msg)                                                             \
    do {                                                                                           \
        if (strcmp((a), (b)) != 0) TEST_FAIL(name, msg);                                           \
    } while (0)

/* ============================================================================
 * Test Runner
 * ============================================================================ */

int main(void) {
    int failures = 0;

    return failures == 0 ? 0 : 1;
}
