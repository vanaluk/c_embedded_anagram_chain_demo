/**
 * @file test_main_freertos.c
 * @brief FreeRTOS test runner main
 */

#include "FreeRTOS.h"
#include "task.h"
#include "test_runner.h"
#include "uart.h"

#define TEST_TASK_STACK_SIZE (4096) /* Words - tests need more stack */
#define TEST_TASK_PRIORITY   (tskIDLE_PRIORITY + 1)

/**
 * @brief FreeRTOS task that runs all tests
 */
static void vTestTask(void *pvParameters)
{
    (void)pvParameters;

    uart_puts("\n[FreeRTOS] Test task started\n");

    int failures = run_all_tests();

    uart_puts("\n[FreeRTOS] Test run complete. ");
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

    /* Task complete */
    vTaskDelete(NULL);
}

int main(void)
{
    /* Initialize UART for output */
    uart_init();

    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  FreeRTOS Test Runner\n");
    uart_puts("  Target: LM3S6965 (Cortex-M3)\n");
    uart_puts("========================================\n");

    /* Create test task */
    BaseType_t xResult = xTaskCreate(vTestTask, "TestRunner",
                                     TEST_TASK_STACK_SIZE, NULL,
                                     TEST_TASK_PRIORITY, NULL);

    if (xResult != pdPASS)
    {
        uart_puts("ERROR: Failed to create test task\n");
        while (1)
        {
        }
    }

    uart_puts("\n[main] Starting FreeRTOS scheduler...\n");

    /* Start scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    uart_puts("ERROR: Scheduler exited\n");
    while (1)
    {
    }

    return 0;
}

/* FreeRTOS hooks */
void vApplicationMallocFailedHook(void)
{
    uart_puts("ERROR: Malloc failed!\n");
    taskDISABLE_INTERRUPTS();
    while (1)
    {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    uart_puts("ERROR: Stack overflow in task: ");
    uart_puts(pcTaskName);
    uart_puts("\n");
    taskDISABLE_INTERRUPTS();
    while (1)
    {
    }
}
