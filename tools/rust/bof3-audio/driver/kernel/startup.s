# SPDX-License-Identifier: MIT
# Owned cold kernel allocation. Call before admitting interrupts or registering
# events, with no active kernel continuation and a separate aligned RAM stack.
# This initializes service state, not game audio state or device registers.
.set noreorder
.set noat
.section .text.audio_kernel_initialize,"ax"
.balign 4
.globl audio_kernel_initialize
.ent audio_kernel_initialize
audio_kernel_initialize:
    lui $t0, %hi(audio_kernel_bss)
    addiu $t0, $t0, %lo(audio_kernel_bss)
    lui $t1, %hi(audio_kernel_state_end)
    addiu $t1, $t1, %lo(audio_kernel_state_end)
.Lclear_state:
    sw $zero, 0($t0)
    addiu $t0, $t0, 4
    bne $t0, $t1, .Lclear_state
    nop

    lui $t0, %hi(.Lchains)
    addiu $t0, $t0, %lo(.Lchains)
    lui $t1, %hi(audio_exception_table)
    addiu $t1, $t1, %lo(audio_exception_table)
    lui $t2, %hi(.Lchains_end)
    addiu $t2, $t2, %lo(.Lchains_end)
.Lcopy_chains:
    lw $t3, 0($t0)
    addiu $t0, $t0, 4
    sw $t3, 0($t1)
    bne $t0, $t2, .Lcopy_chains
    addiu $t1, $t1, 4

    addiu $t0, $zero, 0x100
    lui $t1, %hi(audio_exception_table)
    addiu $t1, $t1, %lo(audio_exception_table)
    sw $t1, 0($t0)
    addiu $t2, $zero, 32
    sw $t2, 4($t0)
    lui $t1, %hi(audio_process)
    addiu $t1, $t1, %lo(audio_process)
    sw $t1, 8($t0)
    addiu $t2, $zero, 4
    sw $t2, 12($t0)
    lui $t2, %hi(audio_thread)
    addiu $t2, $t2, %lo(audio_thread)
    sw $t2, 0($t1)
    sw $t2, 16($t0)
    addiu $t3, $zero, 0x4000
    sw $t3, 0($t2)
    addiu $t2, $zero, 0xc0
    sw $t2, 20($t0)
    lui $t1, %hi(audio_events)
    addiu $t1, $t1, %lo(audio_events)
    sw $t1, 32($t0)
    addiu $t2, $zero, 16 * 28
    sw $t2, 36($t0)

    lui $t0, %hi(audio_exception_hook_pointer)
    sw $zero, %lo(audio_exception_hook_pointer)($t0)
    lui $t0, %hi(audio_exception_stack_pointer)
    lui $t1, %hi(audio_exception_stack_end - 16)
    addiu $t1, $t1, %lo(audio_exception_stack_end - 16)
    sw $t1, %lo(audio_exception_stack_pointer)($t0)
    lui $t0, %hi(audio_irq_registers)
    lui $t1, 0x1f80
    ori $t1, $t1, 0x1070
    sw $t1, %lo(audio_irq_registers)($t0)
    lui $t0, %hi(audio_counter_flags)
    addiu $t0, $t0, %lo(audio_counter_flags)
    addiu $t1, $zero, 4
    addiu $t2, $zero, 1
.Lcounter_flags:
    sw $t2, 0($t0)
    addiu $t1, $t1, -1
    bne $t1, $zero, .Lcounter_flags
    addiu $t0, $t0, 4
    lui $t0, %hi(audio_generic_flags)
    addiu $t0, $t0, %lo(audio_generic_flags)
    addiu $t1, $zero, 11
.Lgeneric_flags:
    sw $zero, 0($t0)
    addiu $t1, $t1, -1
    bne $t1, $zero, .Lgeneric_flags
    addiu $t0, $t0, 4

    lui $t0, %hi(audio_cd_removal_state)
    addiu $t0, $t0, %lo(audio_cd_removal_state)
    sw $zero, 0($t0)
    sw $zero, 4($t0)
    sw $zero, 8($t0)
    sw $zero, 12($t0)
    sw $zero, 16($t0)
    lui $t1, %hi(audio_cd_irq_node)
    addiu $t1, $t1, %lo(audio_cd_irq_node)
    sw $t1, 20($t0)
    lui $t1, %hi(audio_cd_dma_node)
    addiu $t1, $t1, %lo(audio_cd_dma_node)
    sw $t1, 24($t0)
    jr $ra
    addu $v0, $zero, $zero
.end audio_kernel_initialize

.section .rodata.audio_kernel_chains,"a"
.balign 4
.Lchains:
    .word audio_syscall_node, 0
    .word audio_vblank_node, 0
    .word 0, 0
    .word audio_generic_node, 0
    .word 0, 0, audio_syscall_handler, 0
    .word audio_counter_2_node, audio_vblank_acknowledge, audio_vblank_verify, 0
    .word audio_counter_1_node, audio_counter_2_acknowledge, audio_counter_2_verify, 0
    .word audio_counter_0_node, audio_counter_1_acknowledge, audio_counter_1_verify, 0
    .word 0, audio_counter_0_acknowledge, audio_counter_0_verify, 0
    .word 0, 0, audio_generic_irq, 0
.Lchains_end:

.section .bss.audio_kernel_state,"aw",@nobits
.balign 8
.globl audio_kernel_state
audio_kernel_state:
.globl audio_exception_table
audio_exception_table:
    .space 32
.globl audio_syscall_node
audio_syscall_node:
    .space 16
.globl audio_vblank_node
audio_vblank_node:
    .space 16
.globl audio_counter_2_node
audio_counter_2_node:
    .space 16
.globl audio_counter_1_node
audio_counter_1_node:
    .space 16
.globl audio_counter_0_node
audio_counter_0_node:
    .space 16
.globl audio_generic_node
audio_generic_node:
    .space 16
.globl audio_process
audio_process:
    .space 4
.globl audio_thread
audio_thread:
    .space 0xc0
.globl audio_events
audio_events:
    .space 16 * 28
.globl audio_kernel_state_end
audio_kernel_state_end:

.section .bss.audio_kernel_stacks,"aw",@nobits
.balign 8
.globl audio_initialization_stack
audio_initialization_stack:
    .space 1024
.globl audio_initialization_stack_end
audio_initialization_stack_end:
.globl audio_exception_stack
audio_exception_stack:
    .space 4096
.globl audio_exception_stack_end
audio_exception_stack_end:
