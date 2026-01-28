/*
 * assert.h - Custom assert implementation for embedded systems
 *
 * Provides ASSERT macro that:
 * - Outputs error message with file/line info via TRACE
 * - Halts on ARM, exits on PC
 */

#ifndef ASSERT_H_
#define ASSERT_H_

#include "trace.h"

#if defined(PLATFORM_ARM)
#define ASSERT_HALT() \
    do                \
    {                 \
        while (1)     \
        {             \
        }             \
    } while (0)
#else
#include <stdlib.h>
#define ASSERT_HALT() exit(1)
#endif

#define ASSERT(cond)                                                     \
    do                                                                   \
    {                                                                    \
        if (!(cond))                                                     \
        {                                                                \
            TRACE_ERR("ASSERT FAILED: %s at %s:%d", #cond, __FILE__,     \
                      __LINE__);                                         \
            OUTPUT("ASSERT FAILED: %s\n  File: %s\n  Line: %d\n", #cond, \
                   __FILE__, __LINE__);                                  \
            ASSERT_HALT();                                               \
        }                                                                \
    } while (0)

#define ASSERT_MSG(cond, msg)                                                  \
    do                                                                         \
    {                                                                          \
        if (!(cond))                                                           \
        {                                                                      \
            TRACE_ERR("ASSERT FAILED: %s (%s) at %s:%d", #cond, msg, __FILE__, \
                      __LINE__);                                               \
            OUTPUT("ASSERT FAILED: %s\n  Message: %s\n  File: %s\n  Line: "    \
                   "%d\n",                                                     \
                   #cond, msg, __FILE__, __LINE__);                            \
            ASSERT_HALT();                                                     \
        }                                                                      \
    } while (0)

#define ASSERT_NOT_NULL(ptr) ASSERT_MSG((ptr) != NULL, #ptr " is NULL")

#define ASSERT_RANGE(val, min, max)             \
    ASSERT_MSG((val) >= (min) && (val) < (max), \
               #val " out of range [" #min ", " #max ")")

#endif /* ASSERT_H_ */
