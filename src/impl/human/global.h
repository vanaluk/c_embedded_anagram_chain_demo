/*
 * global.h - Common includes and macros for human implementation
 *
 * This file must be included first in all human implementation source files.
 * It ensures correct include order: config.h before anagram_chain.h.
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdint.h>
#include <string.h>

#if !defined(PLATFORM_ARM)
#include <stdio.h>
#include <time.h>
#endif

#if !defined(PLATFORM_ARM) || defined(USE_DYNAMIC_MEMORY)
#include <stdlib.h>
#endif

/* config.h must be included first to override constants in anagram_chain.h */
#include "config.h"
#include "anagram_chain.h"
#include "trace.h"
#include "assert.h"

#define UNUSED(x) ((void)(x))

typedef char WordBuffer[POOL_MAX_WORD_LEN];
typedef size_t ChainPath[MAX_CHAIN_DEPTH];

typedef struct
{
    WordBuffer word;
    WordBuffer signature;
} WordEntry;

typedef struct
{
    ChainPath path;
    WordBuffer candidate;
    WordBuffer temp_sig;
} DfsContext;

typedef struct
{
    ChainPath indices;
    size_t length;
} ChainStorage;

/* Memory pools for dynamic allocation mode */
typedef struct
{
    char *word_pool;
    char *sig_pool;
    size_t pool_size;
    size_t word_used;
    size_t sig_used;
} MemoryPools;

#endif /* GLOBAL_H_ */
