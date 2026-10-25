/* ========================================================================
 * CUSTOM GNU ld LINKER SCRIPT FOR BARE-METAL PDP-11 TARGET
 * Target Architecture: pdp11 (16-bit addressing, usually aout or elf)
 * ======================================================================== */

OUTPUT_ARCH(pdp11)
ENTRY(start)

MEMORY
{
    /* 
     * Base RAM configuration for a standard PDP-11 (e.g., PDP-11/03, 11/23, 11/34).
     * Vector tables occupy 0x0000 - 0x03FF (Addresses 000 to 1776 octal).
     * Standard bare-metal applications typically start at 0x0400 (1000 octal).
     * This allocates 56KB of RAM, leaving the top 8KB (0xE000 - 0xFFFF) 
     * reserved for the Peripheral I/O Page (Peripheral Control Registers / I/O Space).
     */
    /*RAM (rwx) : ORIGIN = 0x0400, LENGTH = 56K - 0x0400*/
    RAM (rwx) : ORIGIN = 0x000, LENGTH = 56K
}

SECTIONS
{
    /* 
     * 1. Code Section 
     * Placed at the very beginning of user-accessible RAM.
     */
    .text :
    {
        _text_start = .;
        *(.text)
        *(.text.*)
        . = ALIGN(2); /* PDP-11 instructions must be word-aligned (2 bytes) */
        _text_end = .;
    } > RAM

    /* 
     * 2. Read-Only Data Section 
     * Contains constants, strings, and immutable variables.
     */
    .rodata :
    {
        _rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(2);
        _rodata_end = .;
    } > RAM

    /* 
     * 3. Read-Write Initialized Data Section 
     * In a bare-metal PDP-11 simulation or ROM-to-RAM system, 
     * this script loads data directly into its execution address.
     */
    .data :
    {
        _data_start = .;
        *(.data)
        *(.data.*)
        . = ALIGN(2);
        _data_end = .;
    } > RAM

    /* 
     * 4. Uninitialized Data Section (Block Started by Symbol)
     * Cleared to zero by the assembly startup code.
     */
    .bss :
    {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(2);
        _bss_end = .;
    } > RAM

    /* 
     * 5. Stack and Heap Definition 
     * On the PDP-11, the stack grows DOWNWARD toward lower memory addresses.
     * We position the initial Stack Pointer (SP) at the very top of user RAM.
     */
    _heap_start = .;
    _stack_top = ORIGIN(RAM) + LENGTH(RAM);

    /* Assertions to verify our code does not overlap into the I/O Page */
    __io_page_start = 0xE000;
    ASSERT(. < __io_page_start, "Error: Binary image overlaps into PDP-11 Peripheral I/O Page!")
}
