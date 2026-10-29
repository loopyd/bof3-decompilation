# SPDX-License-Identifier: MIT
# Owned SPU transfer dispatcher and DMA direction helpers.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Retain complete functions, live caller data and device/event side effects.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x80168960..0x80168BF0
.globl audio_spu_transfer
.ent audio_spu_transfer
audio_spu_transfer:
    sw $a0, 0($sp)
    sw $a1, 4($sp)
    sw $a2, 8($sp)
    sw $a3, 0xc($sp)
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    addiu $s0, $sp, 0x1c
    addiu $a2, $zero, 1
    sw $ra, 0x14($sp)
    beq $a0, $a2, .Lreverb_801689f0
    sw $a0, 0x18($sp)
    slti $v0, $a0, 2
    beqz $v0, .Lreverb_801689a8
    nop
    beqz $a0, .Lreverb_80168a60
    addu $v0, $zero, $zero
    j .Lreverb_80168bdc
    nop
.Lreverb_801689a8:
    addiu $v0, $zero, 2
    beq $a0, $v0, .Lreverb_801689c4
    addiu $v0, $zero, 3
    beq $a0, $v0, .Lreverb_80168acc
    addu $v0, $zero, $zero
    j .Lreverb_80168bdc
    nop
.Lreverb_801689c4:
    lw $a0, 0x1c($sp)
    lui $v0, %hi(audio_spu_transfer_shift)
    lw $v0, %lo(audio_spu_transfer_shift)($v0)
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    srlv $v0, $a0, $v0
    lui $at, %hi(audio_spu_transfer_address)
    sh $v0, %lo(audio_spu_transfer_address)($at)
    sh $v0, 0x1a6($v1)
    j .Lreverb_80168bdc
    addu $v0, $zero, $zero
.Lreverb_801689f0:
    lui $a1, %hi(audio_spu_registers_pointer)
    lw $a1, %lo(audio_spu_registers_pointer)($a1)
    lui $a0, %hi(audio_spu_transfer_address)
    lhu $a0, %lo(audio_spu_transfer_address)($a0)
    lhu $v0, 0x1a6($a1)
    lui $at, %hi(audio_spu_transfer_direction)
    sw $zero, %lo(audio_spu_transfer_direction)($at)
    andi $v0, $v0, 0xffff
    beq $v0, $a0, .Lreverb_80168a38
    addu $v1, $zero, $zero
    addiu $v1, $v1, 1
.Lreverb_80168a1c:
    sltiu $v0, $v1, 0xf01
    beqz $v0, .Lreverb_80168bdc
    addiu $v0, $zero, -2
    lhu $v0, 0x1a6($a1)
    nop
    bne $v0, $a0, .Lreverb_80168a1c
    addiu $v1, $v1, 1
.Lreverb_80168a38:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    nop
    lhu $a0, 0x1aa($v1)
    nop
    andi $v0, $a0, 0xffcf
    ori $a0, $v0, 0x20
    sh $a0, 0x1aa($v1)
    j .Lreverb_80168bdc
    addu $v0, $zero, $zero
.Lreverb_80168a60:
    lui $a1, %hi(audio_spu_registers_pointer)
    lw $a1, %lo(audio_spu_registers_pointer)($a1)
    lui $a0, %hi(audio_spu_transfer_address)
    lhu $a0, %lo(audio_spu_transfer_address)($a0)
    lhu $v0, 0x1a6($a1)
    lui $at, %hi(audio_spu_transfer_direction)
    sw $a2, %lo(audio_spu_transfer_direction)($at)
    andi $v0, $v0, 0xffff
    beq $v0, $a0, .Lreverb_80168aa8
    addu $v1, $zero, $zero
    addiu $v1, $v1, 1
.Lreverb_80168a8c:
    sltiu $v0, $v1, 0xf01
    beqz $v0, .Lreverb_80168bdc
    addiu $v0, $zero, -2
    lhu $v0, 0x1a6($a1)
    nop
    bne $v0, $a0, .Lreverb_80168a8c
    addiu $v1, $v1, 1
.Lreverb_80168aa8:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $a0, 0x1aa($v0)
    nop
    ori $a0, $a0, 0x30
    sh $a0, 0x1aa($v0)
    j .Lreverb_80168bdc
    addu $v0, $zero, $zero
.Lreverb_80168acc:
    lui $v0, %hi(audio_spu_transfer_direction)
    lw $v0, %lo(audio_spu_transfer_direction)($v0)
    nop
    bne $v0, $a2, .Lreverb_80168ae4
    addiu $a0, $zero, 0x20
    addiu $a0, $zero, 0x30
.Lreverb_80168ae4:
    lui $a1, %hi(audio_spu_registers_pointer)
    lw $a1, %lo(audio_spu_registers_pointer)($a1)
    addu $v1, $zero, $zero
    lhu $v0, 0x1aa($a1)
    andi $a0, $a0, 0xffff
    andi $v0, $v0, 0x30
    beq $v0, $a0, .Lreverb_80168b24
    addiu $v1, $v1, 1
.Lreverb_80168b04:
    sltiu $v0, $v1, 0xf01
    beqz $v0, .Lreverb_80168bdc
    addiu $v0, $zero, -2
    lhu $v0, 0x1aa($a1)
    nop
    andi $v0, $v0, 0x30
    bne $v0, $a0, .Lreverb_80168b04
    addiu $v1, $v1, 1
.Lreverb_80168b24:
    lui $v1, %hi(audio_spu_transfer_direction)
    lw $v1, %lo(audio_spu_transfer_direction)($v1)
    addiu $v0, $zero, 1
    bne $v1, $v0, .Lreverb_80168b48
    nop
    jal audio_spu_dma_read_mode
    addiu $s0, $s0, 4
    j .Lreverb_80168b54
    lui $a2, 0x100
.Lreverb_80168b48:
    jal audio_spu_dma_write_mode
    addiu $s0, $s0, 4
    lui $a2, 0x100
.Lreverb_80168b54:
    lw $a0, -4($s0)
    lui $at, %hi(audio_spu_transfer_host_address)
    sw $a0, %lo(audio_spu_transfer_host_address)($at)
    lw $a0, 0($s0)
    lui $a1, %hi(audio_spu_dma_address_pointer)
    lw $a1, %lo(audio_spu_dma_address_pointer)($a1)
    srl $v1, $a0, 6
    andi $v0, $a0, 0x3f
    sltu $v0, $zero, $v0
    lui $a0, %hi(audio_spu_transfer_host_address)
    lw $a0, %lo(audio_spu_transfer_host_address)($a0)
    addu $v1, $v1, $v0
    lui $at, %hi(audio_spu_transfer_blocks)
    sw $v1, %lo(audio_spu_transfer_blocks)($at)
    sw $a0, 0($a1)
    lui $v0, %hi(audio_spu_transfer_blocks)
    lw $v0, %lo(audio_spu_transfer_blocks)($v0)
    lui $v1, %hi(audio_spu_dma_count_pointer)
    lw $v1, %lo(audio_spu_dma_count_pointer)($v1)
    sll $v0, $v0, 0x10
    ori $v0, $v0, 0x10
    sw $v0, 0($v1)
    lui $v1, %hi(audio_spu_transfer_direction)
    lw $v1, %lo(audio_spu_transfer_direction)($v1)
    addiu $v0, $zero, 1
    bne $v1, $v0, .Lreverb_80168bc8
    ori $a2, $a2, 0x201
    lui $a2, 0x100
    ori $a2, $a2, 0x200
.Lreverb_80168bc8:
    lui $v0, %hi(audio_spu_dma_control_pointer)
    lw $v0, %lo(audio_spu_dma_control_pointer)($v0)
    nop
    sw $a2, 0($v0)
    addu $v0, $zero, $zero
.Lreverb_80168bdc:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_spu_transfer

# exe/slus_004_22@0x80168E68..0x80168E94
.globl audio_spu_dma_write_mode
.ent audio_spu_dma_write_mode
audio_spu_dma_write_mode:
    lui $a0, %hi(audio_spu_memory_control_pointer)
    lw $a0, %lo(audio_spu_memory_control_pointer)($a0)
    lui $v1, 0xf0ff
    lw $v0, 0($a0)
    ori $v1, $v1, 0xffff
    and $v0, $v0, $v1
    lui $v1, 0x2000
    or $v0, $v0, $v1
    sw $v0, 0($a0)
    jr $ra
    nop
.end audio_spu_dma_write_mode

# exe/slus_004_22@0x80168E94..0x80168EC0
.globl audio_spu_dma_read_mode
.ent audio_spu_dma_read_mode
audio_spu_dma_read_mode:
    lui $a0, %hi(audio_spu_memory_control_pointer)
    lw $a0, %lo(audio_spu_memory_control_pointer)($a0)
    lui $v1, 0xf0ff
    lw $v0, 0($a0)
    ori $v1, $v1, 0xffff
    and $v0, $v0, $v1
    lui $v1, 0x2200
    or $v0, $v0, $v1
    sw $v0, 0($a0)
    jr $ra
    nop
.end audio_spu_dma_read_mode

.section .data
.balign 4
# exe/slus_004_22@0x80183AB4, 4 bytes.
.globl audio_spu_dma_address_pointer
audio_spu_dma_address_pointer:
    .word 0x1f8010c0
# exe/slus_004_22@0x80183AB8, 4 bytes.
.globl audio_spu_dma_count_pointer
audio_spu_dma_count_pointer:
    .word 0x1f8010c4
# exe/slus_004_22@0x80183ABC, 4 bytes.
.globl audio_spu_dma_control_pointer
audio_spu_dma_control_pointer:
    .word 0x1f8010c8
# exe/slus_004_22@0x80183AC4, 4 bytes.
.globl audio_spu_memory_control_pointer
audio_spu_memory_control_pointer:
    .word 0x1f801014

.section .bss
.balign 4
# exe/slus_004_22@0x80183B04, 4 bytes.
.globl audio_spu_transfer_host_address
audio_spu_transfer_host_address:
    .space 4
# exe/slus_004_22@0x80183B08, 4 bytes.
.globl audio_spu_transfer_blocks
audio_spu_transfer_blocks:
    .space 4
