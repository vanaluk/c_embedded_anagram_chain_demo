/**
 * @file startup.s
 * @brief Startup code for Cortex-M3 (LM3S6965)
 *
 * Provides vector table, reset handler, and basic initialization.
 * Compatible with QEMU lm3s6965evb machine.
 */

    .syntax unified
    .cpu cortex-m3
    .thumb

/* ============================================================================
 * Stack Configuration
 * ============================================================================ */

    .section .stack
    .align 3
    .equ Stack_Size, 0x2000    /* 8KB stack */

    .space Stack_Size
    .global __stack_top
__stack_top:

/* ============================================================================
 * Vector Table
 * ============================================================================ */

    .section .isr_vector, "a"
    .align 2
    .global __isr_vector

__isr_vector:
    .word __stack_top          /* 0x00: Initial Stack Pointer */
    .word Reset_Handler        /* 0x04: Reset Handler */
    .word NMI_Handler          /* 0x08: NMI Handler */
    .word HardFault_Handler    /* 0x0C: Hard Fault Handler */
    .word MemManage_Handler    /* 0x10: MPU Fault Handler */
    .word BusFault_Handler     /* 0x14: Bus Fault Handler */
    .word UsageFault_Handler   /* 0x18: Usage Fault Handler */
    .word 0                    /* 0x1C: Reserved */
    .word 0                    /* 0x20: Reserved */
    .word 0                    /* 0x24: Reserved */
    .word 0                    /* 0x28: Reserved */
    .word SVC_Handler          /* 0x2C: SVCall Handler */
    .word DebugMon_Handler     /* 0x30: Debug Monitor Handler */
    .word 0                    /* 0x34: Reserved */
    .word PendSV_Handler       /* 0x38: PendSV Handler */
    .word SysTick_Handler      /* 0x3C: SysTick Handler */

/* ============================================================================
 * Reset Handler
 * ============================================================================ */

    .section .text
    .align 2
    .global Reset_Handler
    .thumb_func
    .type Reset_Handler, %function

Reset_Handler:
    /* Initialize .data section (copy from Flash to RAM) */
    ldr r0, =__data_start
    ldr r1, =__data_end
    ldr r2, =__data_load
    b .copy_data_check

.copy_data_loop:
    ldr r3, [r2], #4
    str r3, [r0], #4

.copy_data_check:
    cmp r0, r1
    blt .copy_data_loop

    /* Initialize .bss section (zero fill) */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    movs r2, #0
    b .zero_bss_check

.zero_bss_loop:
    str r2, [r0], #4

.zero_bss_check:
    cmp r0, r1
    blt .zero_bss_loop

    /* Call main() */
    bl main

    /* If main returns, loop forever */
.halt:
    b .halt

    .size Reset_Handler, . - Reset_Handler

/* ============================================================================
 * Default Exception Handlers (weak symbols, can be overridden)
 * ============================================================================ */

    .macro def_handler name
    .weak \name
    .thumb_func
    .type \name, %function
\name:
    b .
    .size \name, . - \name
    .endm

    def_handler NMI_Handler
    def_handler HardFault_Handler
    def_handler MemManage_Handler
    def_handler BusFault_Handler
    def_handler UsageFault_Handler
    def_handler SVC_Handler
    def_handler DebugMon_Handler
    def_handler PendSV_Handler
    def_handler SysTick_Handler

    .end
