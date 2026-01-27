/**
 * @file test_main_pc.c
 * @brief PC test runner main
 */

#include "test_runner.h"

int main(void)
{
    int failures = run_all_tests();
    return failures == 0 ? 0 : 1;
}
