OUTPUT_ARCH(mips)
ENTRY(audio_kernel_initialize)
audio_exception_global_pointer = 0;
SECTIONS
{
    . = 0x80020000;
    audio_kernel_image = .;
    .text : {
        *(.text.audio_kernel_initialize)
        *(.text)
        *(.text.audio_counters)
        *(.text.audio_generic)
        *(.text.audio_exception_failure)
    }
    .rodata ALIGN(4) : { *(.rodata.audio_kernel_chains) }
    .data ALIGN(4) : {
        *(.data)
        *(.data.audio_exception_hook)
        *(.data.audio_exception_stack)
        *(.data.audio_irq_registers)
        *(.data.audio_syscall_dispatch)
    }
    audio_kernel_image_end = .;
    .bss ALIGN(8) (NOLOAD) : {
        audio_kernel_bss = .;
        *(.bss)
        *(.bss.audio_sound_auxiliary)
        *(.bss.audio_sound_gains)
        *(.bss.audio_sound_pointers)
        *(.bss.audio_sound_sequences)
        *(.bss.audio_kernel_state)
        *(.bss.audio_kernel_stacks)
        audio_kernel_bss_end = .;
    }
    ASSERT(audio_kernel_bss_end <= 0x80040000,
           "kernel allocation exceeds the owned arena")
    /DISCARD/ : {
        *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.comment) *(.gnu.attributes)
    }
}

# The root uses ORI rather than a sign-extending low-half instruction.
audio_loader_staging_high = ABSOLUTE(audio_loader_staging >> 16);
