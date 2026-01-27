/*
 * config.h - Configuration and static definitions for embedded system
 *
 * This file must be included BEFORE anagram_chain.h to override defaults.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>
#include <stddef.h>

/* Global trace flag */
extern bool global_trace_enable;

/* FNV-1a hash constants */
#define FNV_OFFSET_BASIS 2166136261ul
#define FNV_PRIME        16777619ul

/* ASCII printable range for anagram chars */
#define ASCII_MIN 33
#define ASCII_MAX 126

/* Counting sort array size */
#define CHAR_COUNT_SIZE 128

/*
 * Static memory pool configuration
 * No malloc/free - all memory is pre-allocated at compile time
 *
 * ARM version has reduced limits to fit in embedded memory.
 * PC version can handle larger dictionaries.
 */

#if defined(PLATFORM_ARM)
/* ARM embedded: fits in ~32KB SRAM to leave room for FreeRTOS heap */
#define POOL_MAX_WORDS             64
#define POOL_MAX_WORD_LEN          32
#define POOL_HASH_BUCKETS          (POOL_MAX_WORDS)
#define POOL_MAX_HASH_ENTRIES      64
#define POOL_MAX_INDICES_PER_ENTRY 8
#define POOL_MAX_CHAINS            8
#define POOL_MAX_CHAIN_LEN         16
#else
/* PC: larger limits for stress testing (supports up to 1M words) */
#define POOL_MAX_WORDS             1000000
#define POOL_MAX_WORD_LEN          257
#define POOL_HASH_BUCKETS          (POOL_MAX_WORDS)
#define POOL_MAX_HASH_ENTRIES      (POOL_MAX_WORDS)
#define POOL_MAX_INDICES_PER_ENTRY 256
#define POOL_MAX_CHAINS            16384
#define POOL_MAX_CHAIN_LEN         512
#endif

/*
 * Generic buffer pool for temporary allocations
 * Each buffer can hold the largest structure we need
 */
#define POOL_BUFFER_SIZE  4096
#define POOL_BUFFER_COUNT 32

#define MAX_CHAIN_DEPTH  256
#define SIG_BUFFER_COUNT 8

/*
 * Override anagram_chain.h defaults with human-specific values
 * These must be defined before including anagram_chain.h
 */
#define MAX_WORD_LENGTH  (POOL_MAX_WORD_LEN)
#define INITIAL_CAPACITY (POOL_MAX_WORDS)
#define HASH_TABLE_SIZE  (POOL_HASH_BUCKETS)
#define MAX_CHAINS       (POOL_MAX_CHAINS)

#endif /* CONFIG_H_ */
