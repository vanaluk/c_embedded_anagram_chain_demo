/**
 * @file uart.h
 * @brief UART driver for LM3S6965
 *
 * Simple polled UART driver for QEMU lm3s6965evb.
 * UART0 is connected to QEMU's serial output.
 */

#ifndef UART_H
#define UART_H

/**
 * @brief Initialize UART0
 *
 * Configures UART0 for 115200 baud, 8N1.
 * In QEMU, the baud rate doesn't matter as it's emulated.
 */
void uart_init(void);

/**
 * @brief Send a single character via UART0
 * @param c Character to send
 */
void uart_putc(char c);

/**
 * @brief Send a string via UART0
 * @param s Null-terminated string to send
 */
void uart_puts(const char *s);

/**
 * @brief Send an integer as decimal string via UART0
 * @param n Integer to send
 */
void uart_putint(int n);

#endif /* UART_H */
