/**
 * @file linker.ld
 * @brief Linker script for LM3S6965 (Cortex-M3)
 *
 * Memory map:
 *   Flash: 256KB at 0x00000000
 *   SRAM:  64KB at 0x20000000
 */

/* Entry point */
ENTRY(Reset_Handler)

/* Memory regions */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

/* Section definitions */
SECTIONS
{
    /* Vector table - must be at start of Flash */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* Code section */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        __etext = .;
    } > FLASH

    /* Initialized data (copied from Flash to RAM at startup) */
    .data :
    {
        . = ALIGN(4);
        __data_start = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > SRAM AT > FLASH

    /* Load address for .data in Flash */
    __data_load = LOADADDR(.data);

    /* Uninitialized data (zeroed at startup) */
    .bss :
    {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > SRAM

    /* Stack section */
    .stack :
    {
        . = ALIGN(8);
        *(.stack)
        . = ALIGN(8);
    } > SRAM

    /* Heap (remaining SRAM) */
    .heap :
    {
        . = ALIGN(8);
        __heap_start = .;
        . = ORIGIN(SRAM) + LENGTH(SRAM) - 0x2000; /* Reserve 8KB for stack */
        __heap_end = .;
    } > SRAM

    /* Provide symbols for startup code */
    PROVIDE(__stack_top = ORIGIN(SRAM) + LENGTH(SRAM));
}
