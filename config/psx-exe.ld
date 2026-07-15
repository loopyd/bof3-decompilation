OUTPUT_FORMAT("elf32-littlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)

MEMORY
{
    ram (rwx) : ORIGIN = 0x80010000, LENGTH = 0x1F0000
}

SECTIONS
{
    . = 0x80010000;
    
    .text : {
        *(.text)
        *(.text.*)
    } > ram
    
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } > ram
    
    .data : {
        *(.data)
        *(.data.*)
        *(.sdata)
        *(.sdata.*)
    } > ram
    
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(.sbss)
        *(.sbss.*)
        *(COMMON)
        __bss_end = .;
    } > ram
    
    /DISCARD/ : {
        *(.comment)
        *(.note.*)
        *(.MIPS.abiflags)
        *(.reginfo)
    }
}
