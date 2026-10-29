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
    .rodata ALIGN(4) : {
        *(.rodata.audio_kernel_chains)
        *(.rodata.audio_kernel_vectors)
    }
    .data ALIGN(4) : {
        *(.data)
        *(.data.audio_exception_hook)
        *(.data.audio_exception_stack)
        *(.data.audio_irq_registers)
        *(.data.audio_syscall_dispatch)
    }
    # The shared callback word is also numeric pitch data. Keep its code VMA
    # exact and pack the source bytes in the low image for guest installation.
    audio_sequence_scheduler_load = ABSOLUTE(ALIGN(., 4));
    .audio_sequence_scheduler 0x8016C548 : AT(audio_sequence_scheduler_load) {
        *(.text.audio_sequence_scheduler)
        audio_sequence_scheduler_end = .;
    }
    audio_sequence_scheduler_load_end =
        audio_sequence_scheduler_load + SIZEOF(.audio_sequence_scheduler);
    . = audio_sequence_scheduler_load_end;
    audio_kernel_image_end = .;
    .bss ALIGN(8) (NOLOAD) : AT(ALIGN(audio_kernel_image_end, 8)) {
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
    ASSERT(audio_kernel_bss_end <= 0x80050000,
           "kernel allocation exceeds the owned arena")
    ASSERT(audio_entry_vectors_end - audio_entry_vectors == 32,
           "A0/B0 vector installation size changed")
    ASSERT(audio_sequence_scheduler == 0x8016C548 &&
           audio_sequence_scheduler_end == 0x8016C7BC &&
           SIZEOF(.audio_sequence_scheduler) == 628,
           "scheduler execution address or size changed")
    ASSERT((audio_sequence_scheduler_load & 3) == 0 &&
           audio_sequence_scheduler_load_end == audio_kernel_image_end &&
           audio_sequence_scheduler_load >= ADDR(.data) + SIZEOF(.data) &&
           audio_sequence_scheduler_load_end <= audio_kernel_bss,
           "scheduler payload overlaps initialized data or BSS")
    ASSERT(audio_kernel_bss_end <= audio_sequence_scheduler,
           "scheduler execution range overlaps the low allocation")
    /DISCARD/ : {
        *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.comment) *(.gnu.attributes)
    }
}

# The root uses ORI rather than a sign-extending low-half instruction.
audio_loader_staging_high = ABSOLUTE(audio_loader_staging >> 16);
