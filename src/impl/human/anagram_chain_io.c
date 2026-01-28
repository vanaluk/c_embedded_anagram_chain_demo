/*
 * anagram_chain_io.c - I/O and utility functions
 *
 * Common functions used by both static and dynamic memory implementations.
 * Includes: timing, validation, file I/O, output.
 */

#include "global.h"

/* Defined in anagram_chain_core.c - returns word by index */
extern const char *get_word(size_t idx);

/* Timer functions */

double timer_now(void)
{
#if defined(PLATFORM_ARM)
    TRACE(">> timer_now");
    TRACE("<< timer_now result=0.0 (ARM)");

    return 0.0;
#else
    struct timespec ts;
    double result;

    TRACE(">> timer_now");

    clock_gettime(CLOCK_MONOTONIC, &ts);
    result = ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;

    TRACE("<< timer_now result=%.3f", result);

    return result;
#endif
}

void timer_print(const char *label, double start, double end)
{
    double ms;

    ASSERT_NOT_NULL(label);

    TRACE(">> timer_print label=%s", label);

    ms = end - start;

    if (ms >= 1000.0)
    {
        OUTPUT("%s: %.2f s\n", label, ms / 1000.0);
    }
    else
    {
        OUTPUT("%s: %.3f ms\n", label, ms);
    }

    TRACE("<< timer_print");
}

/* Validation functions */

int is_valid_word(const char *word)
{
    size_t len;
    const char *p;
    int result;

    TRACE(">> is_valid_word word=%s", word ? word : "NULL");

    if (!word || !*word)
    {
        TRACE("<< is_valid_word result=0 (empty)");
        return 0;
    }

    len = 0;
    for (p = word; *p; p++, len++)
    {
        if (*p < 33 || *p > 126)
        {
            TRACE("<< is_valid_word result=0 (invalid char)");
            return 0;
        }
    }

    result = (len < 256);

    TRACE("<< is_valid_word result=%d", result);

    return result;
}

int is_derived_signature(const char *s1, const char *s2)
{
    size_t n1;
    size_t n2;
    size_t i;
    size_t j;
    int skip;
    int result;

    TRACE(">> is_derived_signature");

    if (!s1 || !s2)
    {
        TRACE("<< is_derived_signature result=0 (NULL)");
        return 0;
    }

    n1 = strlen(s1);
    n2 = strlen(s2);

    if (n2 != n1 + 1)
    {
        TRACE("<< is_derived_signature result=0 (len)");
        return 0;
    }

    i = 0;
    j = 0;
    skip = 0;

    while (j < n2)
    {
        if (i < n1 && s1[i] == s2[j])
        {
            i++;
            j++;
        }
        else if (!skip)
        {
            skip = 1;
            j++;
        }
        else
        {
            TRACE("<< is_derived_signature result=0");
            return 0;
        }
    }

    result = (i == n1);

    TRACE("<< is_derived_signature result=%d", result);

    return result;
}

/* File I/O */

int load_dictionary(const char *fname, Dictionary *dict)
{
#if defined(PLATFORM_ARM)
    TRACE(">> load_dictionary fname=%s", fname ? fname : "NULL");

    UNUSED(dict);

    if (!fname)
    {
        TRACE_ERR("fname is NULL");
        return -1;
    }

    TRACE_ERR("file loading not supported on ARM");
    OUTPUT("Error: File loading not supported on ARM\n");

    return -1;
#else
    FILE *f;
    char buf[512];
    int n;
    size_t len;

    TRACE(">> load_dictionary fname=%s", fname ? fname : "NULL");

    if (!fname)
    {
        TRACE_ERR("fname is NULL");
        return -1;
    }

    ASSERT_NOT_NULL(dict);

    f = fopen(fname, "r");
    if (!f)
    {
        TRACE_ERR("cannot open file");
        OUTPUT("Error: Cannot open '%s'\n", fname);
        return -1;
    }

    n = 0;

    while (fgets(buf, sizeof(buf), f))
    {
        len = strlen(buf);

        /* Trim trailing whitespace */
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r' ||
                           buf[len - 1] == ' ' || buf[len - 1] == '\t'))
        {
            buf[--len] = '\0';
        }

        if (len && is_valid_word(buf) && dictionary_add(dict, buf) == 0)
        {
            n++;
        }
    }

    fclose(f);

    TRACE("<< load_dictionary result=%d", n);

    return n;
#endif
}

/* Output functions */

void print_chain(Dictionary *dict, Chain *chain)
{
    size_t i;

    UNUSED(dict);
    ASSERT_NOT_NULL(chain);
    ASSERT_NOT_NULL(chain->indices);

    for (i = 0; i < chain->length; i++)
    {
        OUTPUT("%s", get_word(chain->indices[i]));
        if (i < chain->length - 1)
        {
            OUTPUT("->");
        }
    }

    OUTPUT("\n");
}

void print_results(Dictionary *dict, ChainResults *results)
{
    size_t i;

    TRACE(">> print_results");

    UNUSED(dict);

    if (!results || results->count == 0)
    {
        OUTPUT("No chains found.\n");

        TRACE("<< print_results (no chains)");

        return;
    }

    OUTPUT("\nFound %u chain(s) of length %u:\n", (unsigned)results->count,
           (unsigned)results->max_length);

    for (i = 0; i < results->count; i++)
    {
        print_chain(NULL, &results->chains[i]);
    }

    TRACE("<< print_results");
}

void print_usage(const char *prog)
{
    TRACE(">> print_usage");

#if defined(PLATFORM_ARM)
    UNUSED(prog);

    OUTPUT("Embedded Anagram Chain Demo\n");
    OUTPUT("===========================\n\n");
    OUTPUT("ARM version - words loaded via dictionary_add()\n");
#else
    OUTPUT("Embedded Anagram Chain Demo\n");
    OUTPUT("===========================\n\n");
    OUTPUT("Finds the longest chain of derived anagrams in a dictionary.\n\n");
    OUTPUT("Usage: %s <dictionary_file> <starting_word>\n\n", prog);
    OUTPUT("Arguments:\n");
    OUTPUT("  dictionary_file  Path to dictionary file (one word per line)\n");
    OUTPUT("  starting_word    Word to start the chain from\n\n");
    OUTPUT("Example:\n");
    OUTPUT("  %s words.txt abc\n", prog);
#endif

    TRACE("<< print_usage");
}
