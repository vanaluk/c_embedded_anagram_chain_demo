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

#define UNUSED(x) ((void)(x))

#endif /* GLOBAL_H_ */
