/**
 * @file main_freertos.c
 * @brief FreeRTOS main entry point for LM3S6965
 *
 * Creates a FreeRTOS task that runs the anagram chain algorithm.
 * Demonstrates RTOS integration for embedded systems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "anagram_chain.h"
#include "task.h"
#include "uart.h"

/* ============================================================================
 * Embedded Dictionary
 * ============================================================================
 * Since we can't read files in bare-metal, we embed a test dictionary.
 */

static const char *embedded_words[] = {
    "abc",     "abcd",   "abcde",   "abcdef",   "abck",
    "abcek",   "abcelk", "baclekt", "abcdg",    "abcdgh",
    "abcdghi", "abcdp",  "bafced",  "akjpqwmn", NULL /* Sentinel */
};

static const char *start_word = "abck";

/* ============================================================================
 * Anagram Task
 * ============================================================================
 */

#define ANAGRAM_TASK_STACK_SIZE (2048) /* Words */
#define ANAGRAM_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

/**
 * @brief FreeRTOS task that runs the anagram chain finder
 * @param pvParameters Unused
 */
static void vAnagramTask(void *pvParameters) {
    (void)pvParameters;

    TickType_t xStartTick, xEndTick;

    uart_puts("\n[FreeRTOS Task] Starting anagram chain finder...\n\n");

    /* Create dictionary from embedded words */
    uart_puts("Loading embedded dictionary...\n");
    xStartTick = xTaskGetTickCount();

    Dictionary *dict = dictionary_create(32);
    if (!dict) {
        uart_puts("ERROR: Failed to create dictionary\n");
        vTaskDelete(NULL);
        return;
    }

    int count = 0;
    for (const char **w = embedded_words; *w != NULL; w++) {
        if (dictionary_add(dict, *w) == 0) {
            count++;
        }
    }

    xEndTick = xTaskGetTickCount();
    uart_puts("Words loaded: ");
    uart_putint(count);
    uart_puts(" (");
    uart_putint((int)(xEndTick - xStartTick));
    uart_puts(" ticks)\n");

    /* Verify start word exists */
    if (find_word_index(dict, start_word) < 0) {
        uart_puts("ERROR: Start word not found: ");
        uart_puts(start_word);
        uart_puts("\n");
        dictionary_free(dict);
        vTaskDelete(NULL);
        return;
    }

    /* Build index */
    uart_puts("\nBuilding index...\n");
    xStartTick = xTaskGetTickCount();

    HashTable *index = build_index(dict);
    if (!index) {
        uart_puts("ERROR: Failed to build index\n");
        dictionary_free(dict);
        vTaskDelete(NULL);
        return;
    }

    xEndTick = xTaskGetTickCount();
    uart_puts("Unique signatures: ");
    uart_putint((int)index->entry_count);
    uart_puts(" (");
    uart_putint((int)(xEndTick - xStartTick));
    uart_puts(" ticks)\n");

    /* Find chains */
    uart_puts("\nSearching for chains from '");
    uart_puts(start_word);
    uart_puts("'...\n");
    xStartTick = xTaskGetTickCount();

    ChainResults *results = find_longest_chains(index, dict, start_word);

    xEndTick = xTaskGetTickCount();
    uart_puts("Search completed (");
    uart_putint((int)(xEndTick - xStartTick));
    uart_puts(" ticks)\n");

    /* Print results */
    if (results && results->count > 0) {
        uart_puts("\nFound ");
        uart_putint((int)results->count);
        uart_puts(" chain(s) of length ");
        uart_putint((int)results->max_length);
        uart_puts(":\n");

        for (size_t i = 0; i < results->count && i < 5; i++) {
            Chain *chain = &results->chains[i];
            uart_puts("  ");
            for (size_t j = 0; j < chain->length; j++) {
                uart_puts(dict->words[chain->indices[j]]);
                if (j < chain->length - 1) {
                    uart_puts(" -> ");
                }
            }
            uart_puts("\n");
        }
        if (results->count > 5) {
            uart_puts("  ... and ");
            uart_putint((int)(results->count - 5));
            uart_puts(" more\n");
        }
    } else {
        uart_puts("\nNo chains found.\n");
    }

    /* Cleanup */
    chain_results_free(results);
    hashtable_free(index);
    dictionary_free(dict);

    uart_puts("\n[FreeRTOS Task] Done!\n");
    uart_puts("========================================\n");

    /* Task complete - delete self */
    vTaskDelete(NULL);
}

/* ============================================================================
 * Main Function
 * ============================================================================
 */

int main(void) {
    /* Initialize hardware */
    uart_init();

    /* Banner */
    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  Anagram Chain Finder - FreeRTOS\n");
    uart_puts("  Target: LM3S6965 (Cortex-M3)\n");
    uart_puts("========================================\n");

    /* Create the anagram task */
    BaseType_t xResult =
        xTaskCreate(vAnagramTask,            /* Task function */
                    "Anagram",               /* Task name */
                    ANAGRAM_TASK_STACK_SIZE, /* Stack size (words) */
                    NULL,                    /* Parameters */
                    ANAGRAM_TASK_PRIORITY,   /* Priority */
                    NULL                     /* Task handle (not needed) */
        );

    if (xResult != pdPASS) {
        uart_puts("ERROR: Failed to create task\n");
        while (1) {
        }
    }

    uart_puts("\n[main] Starting FreeRTOS scheduler...\n");

    /* Start the scheduler - this never returns */
    vTaskStartScheduler();

    /* Should never reach here */
    uart_puts("ERROR: Scheduler exited\n");
    while (1) {
    }

    return 0;
}

/* ============================================================================
 * FreeRTOS Memory Allocation Failed Hook
 * ============================================================================
 */

void vApplicationMallocFailedHook(void) {
    uart_puts("ERROR: Malloc failed!\n");
    taskDISABLE_INTERRUPTS();
    while (1) {
    }
}

/* ============================================================================
 * FreeRTOS Stack Overflow Hook
 * ============================================================================
 */

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    uart_puts("ERROR: Stack overflow in task: ");
    uart_puts(pcTaskName);
    uart_puts("\n");
    taskDISABLE_INTERRUPTS();
    while (1) {
    }
}
