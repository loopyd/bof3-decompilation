OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
    .header 0x8000f800 : {
        BYTE(0x50) BYTE(0x53) BYTE(0x2d) BYTE(0x58)
        BYTE(0x20) BYTE(0x45) BYTE(0x58) BYTE(0x45)
        LONG(0) LONG(0)
        LONG(_start) LONG(0)
        LONG(0x80010000) LONG(__load_end - 0x80010000)
        LONG(0) LONG(0) LONG(0) LONG(0)
        LONG(0x80096000) LONG(0)
        . = 0x800;
    }
    .text 0x80010000 : { *(.text.start) *(.text) *(.text.*) }
    .rodata : { *(.rodata) *(.rodata.*) }
    .data : { *(.data) *(.data.*) *(.sdata) }
    .bss (NOLOAD) : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss) *(.bss.*) *(.sbss) *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }
    ASSERT(__bss_end <= 0x80090000, "player assets exceed space below driver and stack")
    __note_call = 0x0c000000 | ((player_note_on >> 2) & 0x03ffffff);
    .runtime 0x80096800 : { *(.runtime) }
    __load_end = ALIGN(., 0x800);
    ASSERT(__load_end <= 0x801f7000, "runtime exceeds original US image range")
    /DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.comment) *(.note*) }
}
