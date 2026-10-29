# SPDX-License-Identifier: MIT
# Owned callback manager, preserving the complete used US instruction family.
# Provenance: exe/slus_004_22@0x801748E4..0x801753C4, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Live tables, delay slots, private stack, diagnostics and re-read loops remain.
# Register indices are caller contracts: IRQ 0..10, VBlank 0..7, DMA 0..6.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x801748E4
.globl audio_callbacks_reset
.ent audio_callbacks_reset
audio_callbacks_reset:
    lui $v0, %hi(audio_callbacks_table_pointer)
    lw $v0, %lo(audio_callbacks_table_pointer)($v0)
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    lw $v0, 0xc($v0)
    nop
    jalr $v0
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_reset

# exe/slus_004_22@0x80174914
.globl audio_callbacks_interrupt
.ent audio_callbacks_interrupt
audio_callbacks_interrupt:
    lui $v0, %hi(audio_callbacks_table_pointer)
    lw $v0, %lo(audio_callbacks_table_pointer)($v0)
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    lw $v0, 8($v0)
    nop
    jalr $v0
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_interrupt

# exe/slus_004_22@0x80174944
.globl audio_callbacks_dma
.ent audio_callbacks_dma
audio_callbacks_dma:
    lui $v0, %hi(audio_callbacks_table_pointer)
    lw $v0, %lo(audio_callbacks_table_pointer)($v0)
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    lw $v0, 4($v0)
    nop
    jalr $v0
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_dma

# exe/slus_004_22@0x80174974
.globl audio_callbacks_vblank_slot4
.ent audio_callbacks_vblank_slot4
audio_callbacks_vblank_slot4:
    addiu $sp, $sp, -0x18
    lui $v0, %hi(audio_callbacks_table_pointer)
    lw $v0, %lo(audio_callbacks_table_pointer)($v0)
    addu $a1, $a0, $zero
    sw $ra, 0x10($sp)
    lw $v0, 0x14($v0)
    nop
    jalr $v0
    addiu $a0, $zero, 4
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_vblank_slot4

# exe/slus_004_22@0x801749A8
.globl audio_callbacks_vblank
.ent audio_callbacks_vblank
audio_callbacks_vblank:
    lui $v0, %hi(audio_callbacks_table_pointer)
    lw $v0, %lo(audio_callbacks_table_pointer)($v0)
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    lw $v0, 0x14($v0)
    nop
    jalr $v0
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_vblank

# exe/slus_004_22@0x801749D8
.globl audio_callbacks_stop
.ent audio_callbacks_stop
audio_callbacks_stop:
    lui $v0, %hi(audio_callbacks_table_pointer)
    lw $v0, %lo(audio_callbacks_table_pointer)($v0)
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    lw $v0, 0x10($v0)
    nop
    jalr $v0
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_stop

# exe/slus_004_22@0x80174A08
.globl audio_callbacks_restart
.ent audio_callbacks_restart
audio_callbacks_restart:
    lui $v0, %hi(audio_callbacks_table_pointer)
    lw $v0, %lo(audio_callbacks_table_pointer)($v0)
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    lw $v0, 0x18($v0)
    nop
    jalr $v0
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_restart

# exe/slus_004_22@0x80174A38
.globl audio_callbacks_active
.ent audio_callbacks_active
audio_callbacks_active:
    lui $v0, %hi(audio_callbacks_state + 2)
    lhu $v0, %lo(audio_callbacks_state + 2)($v0)
    jr $ra
    nop
.end audio_callbacks_active

# exe/slus_004_22@0x80174A48
.globl audio_callbacks_read_mask
.ent audio_callbacks_read_mask
audio_callbacks_read_mask:
    lui $v0, %hi(audio_callbacks_mask_pointer)
    lw $v0, %lo(audio_callbacks_mask_pointer)($v0)
    nop
    lhu $v0, 0($v0)
    jr $ra
    nop
.end audio_callbacks_read_mask

# exe/slus_004_22@0x80174A60
.globl audio_callbacks_exchange_mask
.ent audio_callbacks_exchange_mask
audio_callbacks_exchange_mask:
    lui $v1, %hi(audio_callbacks_mask_pointer)
    lw $v1, %lo(audio_callbacks_mask_pointer)($v1)
    nop
    lhu $v0, 0($v1)
    sh $a0, 0($v1)
    jr $ra
    nop
.end audio_callbacks_exchange_mask

# exe/slus_004_22@0x80174A7C
.globl audio_callbacks_initialize
.ent audio_callbacks_initialize
audio_callbacks_initialize:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    lui $s0, %hi(audio_callbacks_state)
    addiu $s0, $s0, %lo(audio_callbacks_state)
    sw $ra, 0x14($sp)
    lhu $v0, 0($s0)
    nop
    bnez $v0, .Lcallbacks_80174b44
    addu $v0, $zero, $zero
    lui $v1, %hi(audio_callbacks_status_pointer)
    lw $v1, %lo(audio_callbacks_status_pointer)($v1)
    lui $v0, %hi(audio_callbacks_mask_pointer)
    lw $v0, %lo(audio_callbacks_mask_pointer)($v0)
    lui $a1, 0x3333
    sh $zero, 0($v0)
    lhu $v0, 0($v0)
    ori $a1, $a1, 0x3333
    sh $v0, 0($v1)
    lui $v0, %hi(audio_callbacks_priority_pointer)
    lw $v0, %lo(audio_callbacks_priority_pointer)($v0)
    addu $a0, $s0, $zero
    sw $a1, 0($v0)
    jal audio_callbacks_clear_state
    addiu $a1, $zero, 0x41a
    jal .Lcallbacks_setjmp
    addiu $a0, $s0, 0x38
    beqz $v0, .Lcallbacks_80174af4
    nop
    jal audio_callbacks_dispatch
    nop
.Lcallbacks_80174af4:
    lui $s0, %hi(audio_callbacks_state + 0x3c)
    addiu $s0, $s0, %lo(audio_callbacks_state + 0x3c)
    addiu $a0, $s0, -4
    addiu $v0, $s0, 0xfdc
    jal .Lcallbacks_hook_entry
    sw $v0, 0($s0)
    addiu $v0, $zero, 1
    jal audio_callbacks_initialize_vblank
    sh $v0, -0x3c($s0)
    lui $v1, %hi(audio_callbacks_table_pointer)
    lw $v1, %lo(audio_callbacks_table_pointer)($v1)
    jal audio_callbacks_initialize_dma
    sw $v0, 0x14($v1)
    lui $a0, %hi(audio_callbacks_table_pointer)
    lw $a0, %lo(audio_callbacks_table_pointer)($a0)
    jal .Lcallbacks_remove_cd
    sw $v0, 4($a0)
    jal .Lcallbacks_exit_critical
    addiu $s0, $s0, -0x3c
    addu $v0, $s0, $zero
.Lcallbacks_80174b44:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_initialize

# exe/slus_004_22@0x80174B58
.globl audio_callbacks_dispatch
.ent audio_callbacks_dispatch
audio_callbacks_dispatch:
    addiu $sp, $sp, -0x28
    sw $s1, 0x14($sp)
    lui $s1, %hi(audio_callbacks_state)
    addiu $s1, $s1, %lo(audio_callbacks_state)
    sw $ra, 0x24($sp)
    sw $s4, 0x20($sp)
    sw $s3, 0x1c($sp)
    sw $s2, 0x18($sp)
    sw $s0, 0x10($sp)
    lhu $v0, 0($s1)
    nop
    bnez $v0, .Lcallbacks_80174bb4
    nop
    lui $v0, %hi(audio_callbacks_status_pointer)
    lw $v0, %lo(audio_callbacks_status_pointer)($v0)
    nop
    lhu $a1, 0($v0)
    lui $a0, %hi(.Lcallbacks_unexpected)
    addiu $a0, $a0, %lo(.Lcallbacks_unexpected)
    jal .Lcallbacks_printf
    nop
    jal .Lcallbacks_return_exception
    nop
.Lcallbacks_80174bb4:
    lui $a0, %hi(audio_callbacks_status_pointer)
    lw $a0, %lo(audio_callbacks_status_pointer)($a0)
    lui $v1, %hi(audio_callbacks_state + 0x30)
    lhu $v1, %lo(audio_callbacks_state + 0x30)($v1)
    addiu $v0, $zero, 1
    lui $at, %hi(audio_callbacks_state + 2)
    sh $v0, %lo(audio_callbacks_state + 2)($at)
    lui $v0, %hi(audio_callbacks_mask_pointer)
    lw $v0, %lo(audio_callbacks_mask_pointer)($v0)
    lhu $a0, 0($a0)
    lhu $v0, 0($v0)
    and $v1, $v1, $a0
    and $v0, $v0, $v1
    beqz $v0, .Lcallbacks_80174c84
    addu $s0, $v0, $zero
    addiu $s3, $zero, 1
    addiu $s4, $s1, 4
.Lcallbacks_80174bf8:
    beqz $s0, .Lcallbacks_80174c54
    addu $s1, $zero, $zero
    addu $s2, $s4, $zero
.Lcallbacks_80174c04:
    slti $v0, $s1, 0xb
    beqz $v0, .Lcallbacks_80174c54
    andi $v0, $s0, 1
    beqz $v0, .Lcallbacks_80174c40
    sllv $v0, $s3, $s1
    lui $v1, %hi(audio_callbacks_status_pointer)
    lw $v1, %lo(audio_callbacks_status_pointer)($v1)
    nor $v0, $zero, $v0
    sh $v0, 0($v1)
    lw $v0, 0($s2)
    nop
    beqz $v0, .Lcallbacks_80174c40
    nop
    jalr $v0
    nop
.Lcallbacks_80174c40:
    addiu $s2, $s2, 4
    srl $s0, $s0, 1
    andi $v0, $s0, 0xffff
    bnez $v0, .Lcallbacks_80174c04
    addiu $s1, $s1, 1
.Lcallbacks_80174c54:
    lui $a0, %hi(audio_callbacks_status_pointer)
    lw $a0, %lo(audio_callbacks_status_pointer)($a0)
    lui $v1, %hi(audio_callbacks_state + 0x30)
    lhu $v1, %lo(audio_callbacks_state + 0x30)($v1)
    lui $v0, %hi(audio_callbacks_mask_pointer)
    lw $v0, %lo(audio_callbacks_mask_pointer)($v0)
    lhu $a0, 0($a0)
    lhu $v0, 0($v0)
    and $v1, $v1, $a0
    and $v0, $v0, $v1
    bnez $v0, .Lcallbacks_80174bf8
    addu $s0, $v0, $zero
.Lcallbacks_80174c84:
    lui $a1, %hi(audio_callbacks_status_pointer)
    lw $a1, %lo(audio_callbacks_status_pointer)($a1)
    lui $a2, %hi(audio_callbacks_mask_pointer)
    lw $a2, %lo(audio_callbacks_mask_pointer)($a2)
    lhu $v1, 0($a1)
    lhu $v0, 0($a2)
    nop
    and $v0, $v0, $v1
    beqz $v0, .Lcallbacks_80174d04
    nop
    lui $v0, %hi(audio_callbacks_timeout)
    lw $v0, %lo(audio_callbacks_timeout)($v0)
    nop
    addiu $v1, $v0, 1
    slti $v0, $v0, 0x801
    lui $at, %hi(audio_callbacks_timeout)
    sw $v1, %lo(audio_callbacks_timeout)($at)
    bnez $v0, .Lcallbacks_80174d0c
    nop
    lui $a0, %hi(.Lcallbacks_timeout)
    addiu $a0, $a0, %lo(.Lcallbacks_timeout)
    lhu $a1, 0($a1)
    lhu $a2, 0($a2)
    jal .Lcallbacks_printf
    nop
    lui $v0, %hi(audio_callbacks_status_pointer)
    lw $v0, %lo(audio_callbacks_status_pointer)($v0)
    lui $at, %hi(audio_callbacks_timeout)
    sw $zero, %lo(audio_callbacks_timeout)($at)
    sh $zero, 0($v0)
    j .Lcallbacks_80174d0c
    nop
.Lcallbacks_80174d04:
    lui $at, %hi(audio_callbacks_timeout)
    sw $zero, %lo(audio_callbacks_timeout)($at)
.Lcallbacks_80174d0c:
    lui $at, %hi(audio_callbacks_state + 2)
    sh $zero, %lo(audio_callbacks_state + 2)($at)
    jal .Lcallbacks_return_exception
    nop
    lw $ra, 0x24($sp)
    lw $s4, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x28
    jr $ra
    nop
.end audio_callbacks_dispatch

# exe/slus_004_22@0x80174D40
.globl audio_callbacks_register_irq
.ent audio_callbacks_register_irq
audio_callbacks_register_irq:
    addiu $sp, $sp, -0x28
    sw $s1, 0x14($sp)
    addu $s1, $a0, $zero
    sw $s2, 0x18($sp)
    addu $s2, $a1, $zero
    lui $a1, %hi(audio_callbacks_state + 4)
    addiu $a1, $a1, %lo(audio_callbacks_state + 4)
    sll $v0, $s1, 2
    addu $a0, $v0, $a1
    sw $ra, 0x24($sp)
    sw $s4, 0x20($sp)
    sw $s3, 0x1c($sp)
    sw $s0, 0x10($sp)
    lw $s4, 0($a0)
    nop
    beq $s2, $s4, .Lcallbacks_80174e70
    addu $v0, $s4, $zero
    lhu $v0, -4($a1)
    nop
    beqz $v0, .Lcallbacks_80174e70
    addu $v0, $s4, $zero
    lui $v0, %hi(audio_callbacks_mask_pointer)
    lw $v0, %lo(audio_callbacks_mask_pointer)($v0)
    nop
    lhu $v1, 0($v0)
    sh $zero, 0($v0)
    beqz $s2, .Lcallbacks_80174dd0
    andi $s3, $v1, 0xffff
    addiu $v1, $zero, 1
    sllv $v1, $v1, $s1
    sw $s2, 0($a0)
    lhu $v0, 0x2c($a1)
    or $s3, $s3, $v1
    or $v0, $v0, $v1
    j .Lcallbacks_80174df8
    sh $v0, 0x2c($a1)
.Lcallbacks_80174dd0:
    addiu $v0, $zero, 1
    sllv $v0, $v0, $s1
    nor $v0, $zero, $v0
    sw $zero, 0($a0)
    lui $v1, %hi(audio_callbacks_state + 0x30)
    lhu $v1, %lo(audio_callbacks_state + 0x30)($v1)
    and $s3, $s3, $v0
    and $v1, $v1, $v0
    lui $at, %hi(audio_callbacks_state + 0x30)
    sh $v1, %lo(audio_callbacks_state + 0x30)($at)
.Lcallbacks_80174df8:
    bnez $s1, .Lcallbacks_80174e1c
    addiu $v0, $zero, 4
    sltiu $s0, $s2, 1
    jal audio_callbacks_clear_pad
    addu $a0, $s0, $zero
    addiu $a0, $zero, 3
    jal audio_callbacks_clear_counter
    addu $a1, $s0, $zero
    addiu $v0, $zero, 4
.Lcallbacks_80174e1c:
    bne $s1, $v0, .Lcallbacks_80174e34
    addiu $v0, $zero, 5
    addu $a0, $zero, $zero
    jal audio_callbacks_clear_counter
    sltiu $a1, $s2, 1
    addiu $v0, $zero, 5
.Lcallbacks_80174e34:
    bne $s1, $v0, .Lcallbacks_80174e4c
    addiu $v0, $zero, 6
    addiu $a0, $zero, 1
    jal audio_callbacks_clear_counter
    sltiu $a1, $s2, 1
    addiu $v0, $zero, 6
.Lcallbacks_80174e4c:
    bne $s1, $v0, .Lcallbacks_80174e5c
    addiu $a0, $zero, 2
    jal audio_callbacks_clear_counter
    sltiu $a1, $s2, 1
.Lcallbacks_80174e5c:
    lui $v0, %hi(audio_callbacks_mask_pointer)
    lw $v0, %lo(audio_callbacks_mask_pointer)($v0)
    nop
    sh $s3, 0($v0)
    addu $v0, $s4, $zero
.Lcallbacks_80174e70:
    lw $ra, 0x24($sp)
    lw $s4, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x28
    jr $ra
    nop
.end audio_callbacks_register_irq

# exe/slus_004_22@0x80174E94
.globl audio_callbacks_stop_manager
.ent audio_callbacks_stop_manager
audio_callbacks_stop_manager:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    lui $s0, %hi(audio_callbacks_state)
    addiu $s0, $s0, %lo(audio_callbacks_state)
    sw $ra, 0x14($sp)
    lhu $v0, 0($s0)
    nop
    beqz $v0, .Lcallbacks_80174f2c
    addu $v0, $zero, $zero
    jal .Lcallbacks_enter_critical
    nop
    lui $v0, %hi(audio_callbacks_mask_pointer)
    lw $v0, %lo(audio_callbacks_mask_pointer)($v0)
    lui $a0, %hi(audio_callbacks_priority_pointer)
    lw $a0, %lo(audio_callbacks_priority_pointer)($a0)
    lhu $v1, 0($v0)
    lui $at, %hi(audio_callbacks_state + 0x32)
    sh $v1, %lo(audio_callbacks_state + 0x32)($at)
    lw $v1, 0($a0)
    lui $a0, %hi(audio_callbacks_status_pointer)
    lw $a0, %lo(audio_callbacks_status_pointer)($a0)
    lui $at, %hi(audio_callbacks_state + 0x34)
    sw $v1, %lo(audio_callbacks_state + 0x34)($at)
    sh $zero, 0($v0)
    lhu $v0, 0($v0)
    nop
    sh $v0, 0($a0)
    lui $a0, %hi(audio_callbacks_priority_pointer)
    lw $a0, %lo(audio_callbacks_priority_pointer)($a0)
    lui $v1, 0x7777
    lw $v0, 0($a0)
    ori $v1, $v1, 0x7777
    and $v0, $v0, $v1
    sw $v0, 0($a0)
    jal .Lcallbacks_reset_entry
    nop
    addu $v0, $s0, $zero
    sh $zero, 0($v0)
.Lcallbacks_80174f2c:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_stop_manager

# exe/slus_004_22@0x80174F40
.globl audio_callbacks_restart_manager
.ent audio_callbacks_restart_manager
audio_callbacks_restart_manager:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    lui $s0, %hi(audio_callbacks_state)
    addiu $s0, $s0, %lo(audio_callbacks_state)
    sw $ra, 0x14($sp)
    lhu $v0, 0($s0)
    nop
    bnez $v0, .Lcallbacks_80174fb0
    nop
    jal .Lcallbacks_hook_entry
    addiu $a0, $s0, 0x38
    lui $a0, %hi(audio_callbacks_mask_pointer)
    lw $a0, %lo(audio_callbacks_mask_pointer)($a0)
    lui $v1, %hi(audio_callbacks_state + 0x32)
    lhu $v1, %lo(audio_callbacks_state + 0x32)($v1)
    addiu $v0, $zero, 1
    sh $v0, 0($s0)
    sh $v1, 0($a0)
    lui $v1, %hi(audio_callbacks_priority_pointer)
    lw $v1, %lo(audio_callbacks_priority_pointer)($v1)
    lui $v0, %hi(audio_callbacks_state + 0x34)
    lw $v0, %lo(audio_callbacks_state + 0x34)($v0)
    nop
    sw $v0, 0($v1)
    jal .Lcallbacks_exit_critical
    nop
    j .Lcallbacks_80174fb4
    addu $v0, $s0, $zero
.Lcallbacks_80174fb0:
    addu $v0, $zero, $zero
.Lcallbacks_80174fb4:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_restart_manager

# exe/slus_004_22@0x80174FC8
.globl audio_callbacks_clear_state
.ent audio_callbacks_clear_state
audio_callbacks_clear_state:
    addiu $sp, $sp, -8
    beqz $a1, .Lcallbacks_80174fe8
    addiu $v0, $a1, -1
    addiu $v1, $zero, -1
.Lcallbacks_80174fd8:
    sw $zero, 0($a0)
    addiu $v0, $v0, -1
    bne $v0, $v1, .Lcallbacks_80174fd8
    addiu $a0, $a0, 4
.Lcallbacks_80174fe8:
    addiu $sp, $sp, 8
    jr $ra
    nop
.end audio_callbacks_clear_state

# exe/slus_004_22@0x80174FF4
.globl audio_callbacks_initialize_vblank
.ent audio_callbacks_initialize_vblank
audio_callbacks_initialize_vblank:
    addiu $sp, $sp, -0x18
    lui $a0, %hi(audio_callbacks_vblank_table)
    addiu $a0, $a0, %lo(audio_callbacks_vblank_table)
    lui $v1, %hi(audio_callbacks_vblank_mode_pointer)
    lw $v1, %lo(audio_callbacks_vblank_mode_pointer)($v1)
    addiu $v0, $zero, 0x107
    sw $ra, 0x10($sp)
    sw $v0, 0($v1)
    lui $at, %hi(audio_callbacks_vblank_count)
    sw $zero, %lo(audio_callbacks_vblank_count)($at)
    jal audio_callbacks_clear_vblank
    addiu $a1, $zero, 8
    lui $a1, %hi(audio_callbacks_dispatch_vblank)
    addiu $a1, $a1, %lo(audio_callbacks_dispatch_vblank)
    jal audio_callbacks_interrupt
    addu $a0, $zero, $zero
    lui $v0, %hi(audio_callbacks_register_vblank)
    addiu $v0, $v0, %lo(audio_callbacks_register_vblank)
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_initialize_vblank

# exe/slus_004_22@0x8017504C
.globl audio_callbacks_dispatch_vblank
.ent audio_callbacks_dispatch_vblank
audio_callbacks_dispatch_vblank:
    lui $v0, %hi(audio_callbacks_vblank_count)
    lw $v0, %lo(audio_callbacks_vblank_count)($v0)
    addiu $sp, $sp, -0x20
    sw $s1, 0x14($sp)
    addu $s1, $zero, $zero
    sw $s0, 0x10($sp)
    lui $s0, %hi(audio_callbacks_vblank_table)
    addiu $s0, $s0, %lo(audio_callbacks_vblank_table)
    sw $ra, 0x18($sp)
    addiu $v0, $v0, 1
    lui $at, %hi(audio_callbacks_vblank_count)
    sw $v0, %lo(audio_callbacks_vblank_count)($at)
    lui $v0, %hi(audio_callbacks_vblank_count)
    lw $v0, %lo(audio_callbacks_vblank_count)($v0)
.Lcallbacks_80175084:
    lw $v0, 0($s0)
    nop
    beqz $v0, .Lcallbacks_8017509c
    nop
    jalr $v0
    nop
.Lcallbacks_8017509c:
    addiu $s1, $s1, 1
    slti $v0, $s1, 8
    bnez $v0, .Lcallbacks_80175084
    addiu $s0, $s0, 4
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_callbacks_dispatch_vblank

# exe/slus_004_22@0x801750C4
.globl audio_callbacks_register_vblank
.ent audio_callbacks_register_vblank
audio_callbacks_register_vblank:
    lui $v0, %hi(audio_callbacks_vblank_table)
    addiu $v0, $v0, %lo(audio_callbacks_vblank_table)
    sll $a0, $a0, 2
    addu $a0, $a0, $v0
    lw $v0, 0($a0)
    nop
    beq $a1, $v0, .Lcallbacks_801750e8
    nop
    sw $a1, 0($a0)
.Lcallbacks_801750e8:
    jr $ra
    nop
.end audio_callbacks_register_vblank

# exe/slus_004_22@0x801750F0
.globl audio_callbacks_clear_vblank
.ent audio_callbacks_clear_vblank
audio_callbacks_clear_vblank:
    addiu $sp, $sp, -8
    beqz $a1, .Lcallbacks_80175110
    addiu $v0, $a1, -1
    addiu $v1, $zero, -1
.Lcallbacks_80175100:
    sw $zero, 0($a0)
    addiu $v0, $v0, -1
    bne $v0, $v1, .Lcallbacks_80175100
    addiu $a0, $a0, 4
.Lcallbacks_80175110:
    addiu $sp, $sp, 8
    jr $ra
    nop
.end audio_callbacks_clear_vblank

# exe/slus_004_22@0x8017511C
.globl audio_callbacks_initialize_dma
.ent audio_callbacks_initialize_dma
audio_callbacks_initialize_dma:
    addiu $sp, $sp, -0x18
    lui $a0, %hi(audio_callbacks_dma_table)
    addiu $a0, $a0, %lo(audio_callbacks_dma_table)
    sw $ra, 0x10($sp)
    jal audio_callbacks_clear_dma
    addiu $a1, $zero, 8
    addiu $a0, $zero, 3
    lui $v0, %hi(audio_callbacks_dma_control_pointer)
    lw $v0, %lo(audio_callbacks_dma_control_pointer)($v0)
    lui $a1, %hi(audio_callbacks_dispatch_dma)
    addiu $a1, $a1, %lo(audio_callbacks_dispatch_dma)
    sw $zero, 0($v0)
    jal audio_callbacks_interrupt
    nop
    lui $v0, %hi(audio_callbacks_register_dma)
    addiu $v0, $v0, %lo(audio_callbacks_register_dma)
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_callbacks_initialize_dma

# exe/slus_004_22@0x8017516C
.globl audio_callbacks_dispatch_dma
.ent audio_callbacks_dispatch_dma
audio_callbacks_dispatch_dma:
    lui $v0, %hi(audio_callbacks_dma_control_pointer)
    lw $v0, %lo(audio_callbacks_dma_control_pointer)($v0)
    addiu $sp, $sp, -0x30
    sw $ra, 0x28($sp)
    sw $s5, 0x24($sp)
    sw $s4, 0x20($sp)
    sw $s3, 0x1c($sp)
    sw $s2, 0x18($sp)
    sw $s1, 0x14($sp)
    sw $s0, 0x10($sp)
    lw $v0, 0($v0)
    nop
    srl $v0, $v0, 0x18
    andi $s1, $v0, 0x7f
    beqz $s1, .Lcallbacks_80175248
    nop
    addiu $s4, $zero, 1
    lui $s3, 0xff
    ori $s3, $s3, 0xffff
    lui $s5, %hi(audio_callbacks_dma_table)
    addiu $s5, $s5, %lo(audio_callbacks_dma_table)
.Lcallbacks_801751c0:
    beqz $s1, .Lcallbacks_80175224
    addu $s0, $zero, $zero
    addu $s2, $s5, $zero
.Lcallbacks_801751cc:
    slti $v0, $s0, 7
    beqz $v0, .Lcallbacks_80175224
    andi $v0, $s1, 1
    beqz $v0, .Lcallbacks_80175214
    addiu $v0, $s0, 0x18
    lui $a0, %hi(audio_callbacks_dma_control_pointer)
    lw $a0, %lo(audio_callbacks_dma_control_pointer)($a0)
    sllv $v0, $s4, $v0
    lw $v1, 0($a0)
    or $v0, $v0, $s3
    and $v1, $v1, $v0
    sw $v1, 0($a0)
    lw $v0, 0($s2)
    nop
    beqz $v0, .Lcallbacks_80175214
    nop
    jalr $v0
    nop
.Lcallbacks_80175214:
    addiu $s2, $s2, 4
    srl $s1, $s1, 1
    bnez $s1, .Lcallbacks_801751cc
    addiu $s0, $s0, 1
.Lcallbacks_80175224:
    lui $v0, %hi(audio_callbacks_dma_control_pointer)
    lw $v0, %lo(audio_callbacks_dma_control_pointer)($v0)
    nop
    lw $v0, 0($v0)
    nop
    srl $v0, $v0, 0x18
    andi $s1, $v0, 0x7f
    bnez $s1, .Lcallbacks_801751c0
    nop
.Lcallbacks_80175248:
    lui $a1, %hi(audio_callbacks_dma_control_pointer)
    lw $a1, %lo(audio_callbacks_dma_control_pointer)($a1)
    nop
    lw $v0, 0($a1)
    lui $v1, 0xff00
    and $v0, $v0, $v1
    lui $v1, 0x8000
    beq $v0, $v1, .Lcallbacks_80175280
    nop
    lw $v0, 0($a1)
    nop
    andi $v0, $v0, 0x8000
    beqz $v0, .Lcallbacks_801752c8
    nop
.Lcallbacks_80175280:
    lui $a0, %hi(.Lcallbacks_dma_error)
    addiu $a0, $a0, %lo(.Lcallbacks_dma_error)
    lw $a1, 0($a1)
    jal .Lcallbacks_printf
    addu $s0, $zero, $zero
.Lcallbacks_80175294:
    lui $a0, %hi(.Lcallbacks_dma_address)
    addiu $a0, $a0, %lo(.Lcallbacks_dma_address)
    addu $a1, $s0, $zero
    lui $v0, %hi(audio_callbacks_dma_base_pointer)
    lw $v0, %lo(audio_callbacks_dma_base_pointer)($v0)
    sll $v1, $s0, 4
    addu $v1, $v1, $v0
    lw $a2, 0($v1)
    jal .Lcallbacks_printf
    addiu $s0, $s0, 1
    slti $v0, $s0, 7
    bnez $v0, .Lcallbacks_80175294
    nop
.Lcallbacks_801752c8:
    lw $ra, 0x28($sp)
    lw $s5, 0x24($sp)
    lw $s4, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x30
    jr $ra
    nop
.end audio_callbacks_dispatch_dma

# exe/slus_004_22@0x801752F0
.globl audio_callbacks_register_dma
.ent audio_callbacks_register_dma
audio_callbacks_register_dma:
    addu $a2, $a0, $zero
    lui $v1, %hi(audio_callbacks_dma_table)
    addiu $v1, $v1, %lo(audio_callbacks_dma_table)
    sll $v0, $a2, 2
    addu $v1, $v0, $v1
    lw $a3, 0($v1)
    addu $a0, $a1, $zero
    beq $a0, $a3, .Lcallbacks_80175390
    nop
    beqz $a0, .Lcallbacks_80175358
    lui $v0, 0xff
    lui $a1, %hi(audio_callbacks_dma_control_pointer)
    lw $a1, %lo(audio_callbacks_dma_control_pointer)($a1)
    ori $v0, $v0, 0xffff
    sw $a0, 0($v1)
    lw $a0, 0($a1)
    addiu $v1, $a2, 0x10
    and $a0, $a0, $v0
    addiu $v0, $zero, 1
    sllv $v0, $v0, $v1
    lui $v1, 0x80
    or $v0, $v0, $v1
    or $a0, $a0, $v0
    sw $a0, 0($a1)
    j .Lcallbacks_80175390
    nop
.Lcallbacks_80175358:
    lui $a1, %hi(audio_callbacks_dma_control_pointer)
    lw $a1, %lo(audio_callbacks_dma_control_pointer)($a1)
    ori $v0, $v0, 0xffff
    sw $zero, 0($v1)
    lw $v1, 0($a1)
    addiu $a0, $a2, 0x10
    and $v1, $v1, $v0
    lui $v0, 0x80
    or $v1, $v1, $v0
    addiu $v0, $zero, 1
    sllv $v0, $v0, $a0
    nor $v0, $zero, $v0
    and $v1, $v1, $v0
    sw $v1, 0($a1)
.Lcallbacks_80175390:
    jr $ra
    addu $v0, $a3, $zero
.end audio_callbacks_register_dma

# exe/slus_004_22@0x80175398
.globl audio_callbacks_clear_dma
.ent audio_callbacks_clear_dma
audio_callbacks_clear_dma:
    addiu $sp, $sp, -8
    beqz $a1, .Lcallbacks_801753b8
    addiu $v0, $a1, -1
    addiu $v1, $zero, -1
.Lcallbacks_801753a8:
    sw $zero, 0($a0)
    addiu $v0, $v0, -1
    bne $v0, $v1, .Lcallbacks_801753a8
    addiu $a0, $a0, 4
.Lcallbacks_801753b8:
    addiu $sp, $sp, 8
    jr $ra
    nop
.end audio_callbacks_clear_dma

# Original service wrapper at exe/slus_004_22@0x8017E354.
.Lcallbacks_setjmp:
    addiu $t2, $zero, 0xa0
    jr $t2
    addiu $t1, $zero, 0x13
    nop

# Original service wrapper at exe/slus_004_22@0x8017E3E4.
.Lcallbacks_printf:
    addiu $t2, $zero, 0xa0
    jr $t2
    addiu $t1, $zero, 0x3f
    nop

# Original service wrapper at exe/slus_004_22@0x8017ED14.
.Lcallbacks_remove_cd:
    addiu $t2, $zero, 0xa0
    jr $t2
    addiu $t1, $zero, 0x72
    nop

# Original service wrapper at exe/slus_004_22@0x8017EDDC.
.Lcallbacks_return_exception:
    addiu $t2, $zero, 0xb0
    jr $t2
    addiu $t1, $zero, 0x17
    nop

# Original service wrapper at exe/slus_004_22@0x8017EDEC.
.Lcallbacks_reset_entry:
    addiu $t2, $zero, 0xb0
    jr $t2
    addiu $t1, $zero, 0x18
    nop

# Original service wrapper at exe/slus_004_22@0x8017EDFC.
.Lcallbacks_hook_entry:
    addiu $t2, $zero, 0xb0
    jr $t2
    addiu $t1, $zero, 0x19
    nop

# Original service wrapper at exe/slus_004_22@0x8017EE0C.
.Lcallbacks_enter_critical:
    addiu $a0, $zero, 1
    syscall
    jr $ra
    nop

# Original service wrapper at exe/slus_004_22@0x8017EE1C.
.Lcallbacks_exit_critical:
    addiu $a0, $zero, 2
    syscall
    jr $ra
    nop

# Original service wrapper at exe/slus_004_22@0x8017EEBC.
.globl audio_callbacks_clear_pad
audio_callbacks_clear_pad:
    addiu $t2, $zero, 0xb0
    jr $t2
    addiu $t1, $zero, 0x5b
    nop

# Original service wrapper at exe/slus_004_22@0x8017EEDC.
.globl audio_callbacks_clear_counter
audio_callbacks_clear_counter:
    addiu $t2, $zero, 0xc0
    jr $t2
    addiu $t1, $zero, 0xa
    nop

.section .data
.balign 4
.globl audio_callbacks_table
audio_callbacks_table:
    .word .Lcallbacks_version, 0, audio_callbacks_register_irq
    .word audio_callbacks_initialize, audio_callbacks_stop_manager, 0
    .word audio_callbacks_restart_manager, audio_callbacks_state
.globl audio_callbacks_table_pointer
audio_callbacks_table_pointer:
    .word audio_callbacks_table
.globl audio_callbacks_status_pointer
audio_callbacks_status_pointer:
    .word 0x1f801070
.globl audio_callbacks_mask_pointer
audio_callbacks_mask_pointer:
    .word 0x1f801074
.globl audio_callbacks_priority_pointer
audio_callbacks_priority_pointer:
    .word 0x1f8010f0
.globl audio_callbacks_vblank_mode_pointer
audio_callbacks_vblank_mode_pointer:
    .word 0x1f801114
.globl audio_callbacks_dma_control_pointer
audio_callbacks_dma_control_pointer:
    .word 0x1f8010f4
.globl audio_callbacks_dma_base_pointer
audio_callbacks_dma_base_pointer:
    .word 0x1f801080
.Lcallbacks_version:
    .asciz "$Id: intr.c,v 1.74 1996/12/04 07:30:16 makoto Exp $"
.Lcallbacks_unexpected:
    .asciz "unexpected interrupt(%04x)\n"
.Lcallbacks_timeout:
    .asciz "intr timeout(%04x:%04x)\n"
.Lcallbacks_dma_error:
    .asciz "DMA bus error: code=%08x\n"
.Lcallbacks_dma_address:
    .asciz "MADR[%d]=%08x\n"

.section .bss
.balign 8
.globl audio_callbacks_state
audio_callbacks_state:
    .space 0x1068
.globl audio_callbacks_state_end
audio_callbacks_state_end:
.globl audio_callbacks_timeout
audio_callbacks_timeout:
    .space 4
.globl audio_callbacks_vblank_table
audio_callbacks_vblank_table:
    .space 8 * 4
.globl audio_callbacks_vblank_count
audio_callbacks_vblank_count:
    .space 4
.globl audio_callbacks_dma_table
audio_callbacks_dma_table:
    .space 8 * 4
