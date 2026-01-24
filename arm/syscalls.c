/**
 * @file syscalls.c
 * @brief Minimal syscalls for newlib (ARM bare-metal)
 *
 * Provides stubs required by newlib's libc.
 * Only _write is implemented (for printf -> UART).
 */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "uart.h"

/* Heap boundaries (from linker script) */
extern char __heap_start;
extern char __heap_end;
static char *heap_ptr = 0;

/**
 * @brief Write to file descriptor (stdout/stderr -> UART)
 */
int _write(int fd, char *buf, int len) {
    (void)fd; /* Ignore fd, all output goes to UART */

    for (int i = 0; i < len; i++) {
        if (buf[i] == '\n') {
            uart_putc('\r');
        }
        uart_putc(buf[i]);
    }
    return len;
}

/**
 * @brief Allocate heap memory (for malloc)
 */
void *_sbrk(int incr) {
    char *prev_heap;

    if (heap_ptr == 0) {
        heap_ptr = &__heap_start;
    }

    prev_heap = heap_ptr;

    if (heap_ptr + incr > &__heap_end) {
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_ptr += incr;
    return (void *)prev_heap;
}

/**
 * @brief Close file (stub)
 */
int _close(int fd) {
    (void)fd;
    return -1;
}

/**
 * @brief File status (stub)
 */
int _fstat(int fd, struct stat *st) {
    (void)fd;
    st->st_mode = S_IFCHR; /* Character device */
    return 0;
}

/**
 * @brief Check if fd is a terminal (stub)
 */
int _isatty(int fd) {
    (void)fd;
    return 1; /* Always a terminal */
}

/**
 * @brief Seek file (stub)
 */
int _lseek(int fd, int offset, int whence) {
    (void)fd;
    (void)offset;
    (void)whence;
    return 0;
}

/**
 * @brief Read from file (stub)
 */
int _read(int fd, char *buf, int len) {
    (void)fd;
    (void)buf;
    (void)len;
    return 0; /* EOF */
}

/**
 * @brief Exit program
 */
void _exit(int status) {
    (void)status;
    while (1) {
        /* Halt */
    }
}

/**
 * @brief Kill process (stub)
 */
int _kill(int pid, int sig) {
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

/**
 * @brief Get process ID (stub)
 */
int _getpid(void) { return 1; }
