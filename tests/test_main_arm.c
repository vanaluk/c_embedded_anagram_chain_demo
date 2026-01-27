/**
 * @file test_main_arm.c
 * @brief ARM bare-metal test runner main
 */

#include "test_runner.h"
#include "uart.h"

int main(void)
{
    /* Initialize UART for output */
    uart_init();

    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  ARM Bare-metal Test Runner\n");
    uart_puts("  Target: LM3S6965 (Cortex-M3)\n");
    uart_puts("========================================\n");

    int failures = run_all_tests();

    uart_puts("\nTest run complete. ");
    if (failures == 0)
    {
        uart_puts("SUCCESS!\n");
    }
    else
    {
        uart_puts("FAILURES: ");
        uart_putint(failures);
        uart_puts("\n");
    }

    /* Halt - use Ctrl+A X to exit QEMU */
    while (1)
    {
        /* Loop forever */
    }

    return failures == 0 ? 0 : 1;
}
