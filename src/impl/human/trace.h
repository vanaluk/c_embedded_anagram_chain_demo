/*
 * trace.h - Tracing support for debugging
 *
 * Usage:
 *   TRACE(">> func_name param=%d", param);
 *   TRACE("<< func_name result=%d", result);
 *   TRACE_ERR("func_name: error message");
 *
 * Enable/disable via global_trace_enable flag.
 */

#ifndef TRACE_H_
#define TRACE_H_

#include <stdbool.h>
#include <stdio.h>

extern bool global_trace_enable;

#define TRACE_BUF_SIZE 256

#if defined(PLATFORM_ARM)
/* ARM platform - use UART for output */
#include "uart.h"

/* Tick counter in microseconds (updated before each TRACE) */
extern volatile unsigned int g_trace_tick_us;

/* Update trace timer - call this before using g_trace_tick_us */
void trace_update_time(void);

#define TRACE(fmt, ...)                                                  \
    do                                                                   \
    {                                                                    \
        if (global_trace_enable)                                         \
        {                                                                \
            char _tbuf[TRACE_BUF_SIZE];                                  \
            trace_update_time();                                         \
            snprintf(_tbuf, TRACE_BUF_SIZE, "[%u.%03u %s:%d] " fmt "\n", \
                     g_trace_tick_us / 1000, g_trace_tick_us % 1000,     \
                     TRACE_FILE, __LINE__, ##__VA_ARGS__);               \
            uart_puts(_tbuf);                                            \
        }                                                                \
    } while (0)

#define OUTPUT(fmt, ...)                                     \
    do                                                       \
    {                                                        \
        char _obuf[TRACE_BUF_SIZE];                          \
        snprintf(_obuf, TRACE_BUF_SIZE, fmt, ##__VA_ARGS__); \
        uart_puts(_obuf);                                    \
    } while (0)

#else
/* PC platform - use stdio */
#include <time.h>

/* Format current UTC time as HH:MM:SS.mmm */
static inline void trace_utc_time(char *buf, int bufsize)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *tm = gmtime(&ts.tv_sec);
    int ms = (int)(ts.tv_nsec / 1000000);
    snprintf(buf, bufsize, "%02d:%02d:%02d.%03d", tm->tm_hour, tm->tm_min,
             tm->tm_sec, ms);
}

#define TRACE(fmt, ...)                                                       \
    do                                                                        \
    {                                                                         \
        if (global_trace_enable)                                              \
        {                                                                     \
            char _tbuf[TRACE_BUF_SIZE];                                       \
            char _timebuf[16];                                                \
            trace_utc_time(_timebuf, sizeof(_timebuf));                       \
            snprintf(_tbuf, TRACE_BUF_SIZE, "[%s %s:%d] " fmt "\n", _timebuf, \
                     TRACE_FILE, __LINE__, ##__VA_ARGS__);                    \
            fputs(_tbuf, stdout);                                             \
        }                                                                     \
    } while (0)

#define OUTPUT(fmt, ...) printf(fmt, ##__VA_ARGS__)

#endif

/* Extract filename from path */
#define TRACE_FILE                \
    ({                            \
        const char *f = __FILE__; \
        const char *p = f;        \
        while (*p)                \
        {                         \
            if (*p == '/')        \
                f = p + 1;        \
            p++;                  \
        }                         \
        f;                        \
    })

#define TRACE_ERR(fmt, ...)                                   \
    do                                                        \
    {                                                         \
        if (global_trace_enable)                              \
        {                                                     \
            TRACE("ERROR %s: " fmt, __func__, ##__VA_ARGS__); \
        }                                                     \
    } while (0)

#endif /* TRACE_H_ */
