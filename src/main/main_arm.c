/**
 * @file main_arm.c
 * @brief ARM bare-metal main entry point for LM3S6965
 *
 * Initializes UART and runs the anagram chain algorithm.
 * Output is sent via UART0 to QEMU's serial console.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "anagram_chain.h"
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
 * Timer Implementation for ARM
 * ============================================================================
 * Simple cycle counter using SysTick (approximate timing).
 */

#define SYSTICK_CTRL (*(volatile unsigned int *)0xE000E010)
#define SYSTICK_LOAD (*(volatile unsigned int *)0xE000E014)
#define SYSTICK_VAL  (*(volatile unsigned int *)0xE000E018)

#define SYSTICK_ENABLE  (1 << 0)
#define SYSTICK_TICKINT (1 << 1) /* Enable interrupt */
#define SYSTICK_CLKSRC  (1 << 2) /* Use processor clock */

/* LM3S6965 runs at 12 MHz in QEMU */
#define CPU_FREQ_HZ  12000000
#define SYSTICK_1MS  (CPU_FREQ_HZ / 1000)
#define TICKS_PER_US (CPU_FREQ_HZ / 1000000) /* 12 ticks per microsecond */

static volatile unsigned int systick_ms = 0;

/* Trace tick counter in microseconds (declared in trace.h) */
extern volatile unsigned int g_trace_tick_us;

void SysTick_Handler(void)
{
    systick_ms++;
}

/* Get current time in microseconds */
static unsigned int timer_arm_get_us(void)
{
    unsigned int ms, val;
    /* Read atomically */
    do
    {
        ms = systick_ms;
        val = SYSTICK_VAL;
    } while (ms != systick_ms);

    /* Calculate microseconds within current ms */
    unsigned int us_in_ms = (SYSTICK_1MS - 1 - val) / TICKS_PER_US;
    return ms * 1000 + us_in_ms;
}

/* Update trace counter - called by TRACE macro */
void trace_update_time(void)
{
    g_trace_tick_us = timer_arm_get_us();
}

static void timer_arm_init(void)
{
    SYSTICK_LOAD = SYSTICK_1MS - 1; /* 1ms interval */
    SYSTICK_VAL = 0;
    SYSTICK_CTRL = SYSTICK_ENABLE | SYSTICK_TICKINT | SYSTICK_CLKSRC;
}

static unsigned int timer_arm_get_ms(void)
{
    return systick_ms;
}

/* ============================================================================
 * ARM Main Function
 * ============================================================================
 */

int main(void)
{
    /* Initialize hardware */
    uart_init();
    timer_arm_init();

    /* Banner */
    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  Embedded Anagram Chain Demo - ARM Bare-metal\n");
    uart_puts("  Target: LM3S6965 (Cortex-M3)\n");
    uart_puts("========================================\n\n");

    /* Create dictionary from embedded words */
    uart_puts("Loading embedded dictionary...\n");
    unsigned int start_ticks = timer_arm_get_ms();

    Dictionary *dict = dictionary_create(32);
    if (!dict)
    {
        uart_puts("ERROR: Failed to create dictionary\n");
        return 1;
    }

    int count = 0;
    for (const char **w = embedded_words; *w != NULL; w++)
    {
        if (dictionary_add(dict, *w) == 0)
        {
            count++;
        }
    }

    unsigned int load_ticks = timer_arm_get_ms() - start_ticks;
    uart_puts("Words loaded: ");
    uart_putint(count);
    uart_puts(" (");
    uart_putint(load_ticks);
    uart_puts(" ticks)\n");

    /* Verify start word exists */
    if (find_word_index(dict, start_word) < 0)
    {
        uart_puts("ERROR: Start word not found: ");
        uart_puts(start_word);
        uart_puts("\n");
        dictionary_free(dict);
        return 1;
    }

    /* Build index */
    uart_puts("\nBuilding index...\n");
    start_ticks = timer_arm_get_ms();

    HashTable *index = build_index(dict);
    if (!index)
    {
        uart_puts("ERROR: Failed to build index\n");
        dictionary_free(dict);
        return 1;
    }

    unsigned int index_ticks = timer_arm_get_ms() - start_ticks;
    uart_puts("Unique signatures: ");
    uart_putint((int)index->entry_count);
    uart_puts(" (");
    uart_putint(index_ticks);
    uart_puts(" ticks)\n");

    /* Find chains */
    uart_puts("\nSearching for chains from '");
    uart_puts(start_word);
    uart_puts("'...\n");
    start_ticks = timer_arm_get_ms();

    ChainResults *results = find_longest_chains(index, dict, start_word);

    unsigned int search_ticks = timer_arm_get_ms() - start_ticks;
    uart_puts("Search completed (");
    uart_putint(search_ticks);
    uart_puts(" ticks)\n");

    /* Print results */
    if (results && results->count > 0)
    {
        uart_puts("\nFound ");
        uart_putint((int)results->count);
        uart_puts(" chain(s) of length ");
        uart_putint((int)results->max_length);
        uart_puts(":\n");

        for (size_t i = 0; i < results->count && i < 5; i++)
        {
            Chain *chain = &results->chains[i];
            uart_puts("  ");
            for (size_t j = 0; j < chain->length; j++)
            {
                uart_puts(dict->words[chain->indices[j]]);
                if (j < chain->length - 1)
                {
                    uart_puts(" -> ");
                }
            }
            uart_puts("\n");
        }
        if (results->count > 5)
        {
            uart_puts("  ... and ");
            uart_putint((int)(results->count - 5));
            uart_puts(" more\n");
        }
    }
    else
    {
        uart_puts("\nNo chains found.\n");
    }

    /* Cleanup */
    chain_results_free(results);
    hashtable_free(index);
    dictionary_free(dict);

    uart_puts("\n========================================\n");
    uart_puts("  Done!\n");
    uart_puts("========================================\n");

    /* Halt (in QEMU, use Ctrl+A X to exit) */
    while (1)
    {
        /* Loop forever */
    }

    return 0;
}
