/**
 * @file uart.c
 * @brief UART driver implementation for LM3S6965
 *
 * LM3S6965 UART0 registers (directly from datasheet):
 *   UART0_DR   = 0x4000C000 (Data Register)
 *   UART0_FR   = 0x4000C018 (Flag Register)
 *   UART0_IBRD = 0x4000C024 (Integer Baud-Rate Divisor)
 *   UART0_FBRD = 0x4000C028 (Fractional Baud-Rate Divisor)
 *   UART0_LCRH = 0x4000C02C (Line Control)
 *   UART0_CTL  = 0x4000C030 (Control)
 */

#include "uart.h"

#include <stdint.h>

/* UART0 Register addresses */
#define UART0_BASE 0x4000C000
#define UART0_DR (*(volatile uint32_t *)(UART0_BASE + 0x000))
#define UART0_FR (*(volatile uint32_t *)(UART0_BASE + 0x018))
#define UART0_IBRD (*(volatile uint32_t *)(UART0_BASE + 0x024))
#define UART0_FBRD (*(volatile uint32_t *)(UART0_BASE + 0x028))
#define UART0_LCRH (*(volatile uint32_t *)(UART0_BASE + 0x02C))
#define UART0_CTL (*(volatile uint32_t *)(UART0_BASE + 0x030))

/* Flag Register bits */
#define UART_FR_TXFF (1 << 5) /* TX FIFO full */
#define UART_FR_RXFE (1 << 4) /* RX FIFO empty */

/* Line Control bits */
#define UART_LCRH_WLEN_8 (0x3 << 5) /* 8-bit word length */
#define UART_LCRH_FEN (1 << 4)      /* Enable FIFOs */

/* Control Register bits */
#define UART_CTL_UARTEN (1 << 0) /* UART enable */
#define UART_CTL_TXE (1 << 8)    /* TX enable */
#define UART_CTL_RXE (1 << 9)    /* RX enable */

void uart_init(void) {
    /* Disable UART during configuration */
    UART0_CTL = 0;

    /*
     * Baud rate calculation (not critical for QEMU):
     * BRD = UART_CLK / (16 * Baud)
     * For 115200 @ 12.5MHz: BRD = 12500000 / (16 * 115200) = 6.78
     * IBRD = 6, FBRD = 0.78 * 64 = 50
     *
     * In QEMU these values don't matter, but we set them anyway.
     */
    UART0_IBRD = 6;
    UART0_FBRD = 50;

    /* 8 data bits, no parity, 1 stop bit, enable FIFOs */
    UART0_LCRH = UART_LCRH_WLEN_8 | UART_LCRH_FEN;

    /* Enable UART, TX, and RX */
    UART0_CTL = UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE;
}

void uart_putc(char c) {
    /* Wait until TX FIFO has space */
    while (UART0_FR & UART_FR_TXFF) {
        /* Spin */
    }
    UART0_DR = (uint32_t)c;
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r'); /* Add CR before LF for terminal compatibility */
        }
        uart_putc(*s++);
    }
}

void uart_putint(int n) {
    char buf[12]; /* Enough for -2147483648 */
    char *p = buf + sizeof(buf) - 1;
    int neg = 0;

    *p = '\0';

    if (n < 0) {
        neg = 1;
        n = -n;
    }

    if (n == 0) {
        *--p = '0';
    } else {
        while (n > 0) {
            *--p = '0' + (n % 10);
            n /= 10;
        }
    }

    if (neg) {
        *--p = '-';
    }

    uart_puts(p);
}
