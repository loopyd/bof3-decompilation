# SPDX-License-Identifier: MIT
# Owned seven-slot layout initialization and its mutable layout/state storage.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# The low selector byte must identify row 0, 1 or 2. Tables remain live.
.set noreorder
.set noat
.section .text
.balign 4
# exe/slus_004_22@0x80161808..0x80161B24
.globl audio_loader_initialize_layout
.ent audio_loader_initialize_layout
audio_loader_initialize_layout:
    # ORI needs the unadjusted upper half supplied by the linker.
    lui $v0, %lo(audio_loader_staging_high)
    andi $a0, $a0, 0xff
    sll $a1, $a0, 3
    subu $a1, $a1, $a0
    sll $a1, $a1, 2
    lui $at, %hi(audio_loader_slots + 0x10)
    sh $zero, %lo(audio_loader_slots + 0x10)($at)
    lui $at, %hi(audio_loader_header_capacities)
    addu $at, $at, $a1
    lw $v1, %lo(audio_loader_header_capacities)($at)
    ori $v0, $v0, %lo(audio_loader_staging)
    lui $at, %hi(audio_loader_header_pointers)
    sw $v0, %lo(audio_loader_header_pointers)($at)
    lui $at, %hi(audio_loader_slots + 0x4)
    sw $v0, %lo(audio_loader_slots + 0x4)($at)
    addu $v1, $v1, $v0
    lui $at, %hi(audio_loader_slots + 0xc)
    sw $v1, %lo(audio_loader_slots + 0xc)($at)
    lui $at, %hi(audio_loader_spu_bases)
    addu $at, $at, $a1
    lw $a0, %lo(audio_loader_spu_bases)($at)
    lui $v0, %hi(audio_loader_auxiliary)
    addiu $v0, $v0, %lo(audio_loader_auxiliary)
    lui $at, %hi(audio_loader_slots + 0x8)
    sw $v0, %lo(audio_loader_slots + 0x8)($at)
    addiu $v0, $zero, 1
    lui $at, %hi(audio_loader_sequence_pointers)
    sw $v1, %lo(audio_loader_sequence_pointers)($at)
    lui $at, %hi(audio_loader_slots + 0x24)
    sh $v0, %lo(audio_loader_slots + 0x24)($at)
    lui $at, %hi(audio_loader_slots)
    sw $a0, %lo(audio_loader_slots)($at)
    lui $at, %hi(audio_loader_sequence_capacities)
    addu $at, $at, $a1
    lw $v0, %lo(audio_loader_sequence_capacities)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x58)
    addu $at, $at, $a1
    lw $a0, %lo(audio_loader_spu_bases + 0x58)($at)
    addu $v1, $v1, $v0
    addu $a0, $v1, $a0
    lui $at, %hi(audio_loader_header_pointers + 0x4)
    sw $v1, %lo(audio_loader_header_pointers + 0x4)($at)
    lui $at, %hi(audio_loader_slots + 0x20)
    sw $a0, %lo(audio_loader_slots + 0x20)($at)
    lui $at, %hi(audio_loader_slots + 0x18)
    sw $v1, %lo(audio_loader_slots + 0x18)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x4)
    addu $at, $at, $a1
    lw $v1, %lo(audio_loader_spu_bases + 0x4)($at)
    lui $v0, %hi(audio_loader_auxiliary + 0x7c)
    addiu $v0, $v0, %lo(audio_loader_auxiliary + 0x7c)
    lui $at, %hi(audio_loader_slots + 0x1c)
    sw $v0, %lo(audio_loader_slots + 0x1c)($at)
    addiu $v0, $zero, 2
    lui $at, %hi(audio_loader_header_pointers + 0x20)
    sw $a0, %lo(audio_loader_header_pointers + 0x20)($at)
    lui $at, %hi(audio_loader_slots + 0x38)
    sh $v0, %lo(audio_loader_slots + 0x38)($at)
    lui $at, %hi(audio_loader_slots + 0x14)
    sw $v1, %lo(audio_loader_slots + 0x14)($at)
    lui $at, %hi(audio_loader_spu_bases + 0xac)
    addu $at, $at, $a1
    lw $v0, %lo(audio_loader_spu_bases + 0xac)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x5c)
    addu $at, $at, $a1
    lw $v1, %lo(audio_loader_spu_bases + 0x5c)($at)
    addu $a0, $a0, $v0
    addu $v1, $a0, $v1
    lui $at, %hi(audio_loader_header_pointers + 0x8)
    sw $a0, %lo(audio_loader_header_pointers + 0x8)($at)
    lui $at, %hi(audio_loader_slots + 0x34)
    sw $v1, %lo(audio_loader_slots + 0x34)($at)
    lui $at, %hi(audio_loader_slots + 0x2c)
    sw $a0, %lo(audio_loader_slots + 0x2c)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x8)
    addu $at, $at, $a1
    lw $a0, %lo(audio_loader_spu_bases + 0x8)($at)
    lui $v0, %hi(audio_loader_auxiliary + 0xf8)
    addiu $v0, $v0, %lo(audio_loader_auxiliary + 0xf8)
    lui $at, %hi(audio_loader_slots + 0x30)
    sw $v0, %lo(audio_loader_slots + 0x30)($at)
    addiu $v0, $zero, 3
    lui $at, %hi(audio_loader_header_pointers + 0x24)
    sw $v1, %lo(audio_loader_header_pointers + 0x24)($at)
    lui $at, %hi(audio_loader_slots + 0x4c)
    sh $v0, %lo(audio_loader_slots + 0x4c)($at)
    lui $at, %hi(audio_loader_slots + 0x28)
    sw $a0, %lo(audio_loader_slots + 0x28)($at)
    lui $at, %hi(audio_loader_spu_bases + 0xb0)
    addu $at, $at, $a1
    lw $v0, %lo(audio_loader_spu_bases + 0xb0)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x60)
    addu $at, $at, $a1
    lw $a0, %lo(audio_loader_spu_bases + 0x60)($at)
    addu $v1, $v1, $v0
    addu $a0, $v1, $a0
    lui $at, %hi(audio_loader_header_pointers + 0xc)
    sw $v1, %lo(audio_loader_header_pointers + 0xc)($at)
    lui $at, %hi(audio_loader_header_pointers + 0x28)
    sw $a0, %lo(audio_loader_header_pointers + 0x28)($at)
    lui $at, %hi(audio_loader_slots + 0x48)
    sw $a0, %lo(audio_loader_slots + 0x48)($at)
    lui $at, %hi(audio_loader_slots + 0x40)
    sw $v1, %lo(audio_loader_slots + 0x40)($at)
    lui $at, %hi(audio_loader_spu_bases + 0xc)
    addu $at, $at, $a1
    lw $v1, %lo(audio_loader_spu_bases + 0xc)($at)
    lui $v0, %hi(audio_loader_auxiliary + 0x174)
    addiu $v0, $v0, %lo(audio_loader_auxiliary + 0x174)
    lui $at, %hi(audio_loader_slots + 0x44)
    sw $v0, %lo(audio_loader_slots + 0x44)($at)
    addiu $v0, $zero, 4
    lui $at, %hi(audio_loader_slots + 0x60)
    sh $v0, %lo(audio_loader_slots + 0x60)($at)
    lui $at, %hi(audio_loader_slots + 0x3c)
    sw $v1, %lo(audio_loader_slots + 0x3c)($at)
    lui $at, %hi(audio_loader_spu_bases + 0xb4)
    addu $at, $at, $a1
    lw $v0, %lo(audio_loader_spu_bases + 0xb4)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x64)
    addu $at, $at, $a1
    lw $t0, %lo(audio_loader_spu_bases + 0x64)($at)
    addu $a0, $a0, $v0
    addu $t0, $a0, $t0
    lui $at, %hi(audio_loader_slots + 0x5c)
    sw $t0, %lo(audio_loader_slots + 0x5c)($at)
    lui $at, %hi(audio_loader_slots + 0x54)
    sw $a0, %lo(audio_loader_slots + 0x54)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x10)
    addu $at, $at, $a1
    lw $v1, %lo(audio_loader_spu_bases + 0x10)($at)
    lui $v0, %hi(audio_loader_auxiliary + 0x1f0)
    addiu $v0, $v0, %lo(audio_loader_auxiliary + 0x1f0)
    lui $at, %hi(audio_loader_slots + 0x58)
    sw $v0, %lo(audio_loader_slots + 0x58)($at)
    addiu $v0, $zero, 5
    lui $at, %hi(audio_loader_slots + 0x74)
    sh $v0, %lo(audio_loader_slots + 0x74)($at)
    lui $at, %hi(audio_loader_slots + 0x50)
    sw $v1, %lo(audio_loader_slots + 0x50)($at)
    lui $at, %hi(audio_loader_spu_bases + 0xb8)
    addu $at, $at, $a1
    lw $a2, %lo(audio_loader_spu_bases + 0xb8)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x68)
    addu $at, $at, $a1
    lw $a3, %lo(audio_loader_spu_bases + 0x68)($at)
    addu $a2, $t0, $a2
    addu $a3, $a2, $a3
    lui $at, %hi(audio_loader_slots + 0x70)
    sw $a3, %lo(audio_loader_slots + 0x70)($at)
    lui $at, %hi(audio_loader_slots + 0x68)
    sw $a2, %lo(audio_loader_slots + 0x68)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x14)
    addu $at, $at, $a1
    lw $v1, %lo(audio_loader_spu_bases + 0x14)($at)
    lui $v0, %hi(audio_loader_auxiliary + 0x26c)
    addiu $v0, $v0, %lo(audio_loader_auxiliary + 0x26c)
    lui $at, %hi(audio_loader_slots + 0x6c)
    sw $v0, %lo(audio_loader_slots + 0x6c)($at)
    addiu $v0, $zero, 6
    lui $at, %hi(audio_loader_slots + 0x88)
    sh $v0, %lo(audio_loader_slots + 0x88)($at)
    lui $at, %hi(audio_loader_slots + 0x64)
    sw $v1, %lo(audio_loader_slots + 0x64)($at)
    lui $at, %hi(audio_loader_spu_bases + 0xbc)
    addu $at, $at, $a1
    lw $v0, %lo(audio_loader_spu_bases + 0xbc)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x6c)
    addu $at, $at, $a1
    lw $v1, %lo(audio_loader_spu_bases + 0x6c)($at)
    addu $v0, $a3, $v0
    addu $v1, $v0, $v1
    lui $at, %hi(audio_loader_slots + 0x84)
    sw $v1, %lo(audio_loader_slots + 0x84)($at)
    lui $at, %hi(audio_loader_slots + 0x7c)
    sw $v0, %lo(audio_loader_slots + 0x7c)($at)
    lui $at, %hi(audio_loader_spu_bases + 0x18)
    addu $at, $at, $a1
    lw $a1, %lo(audio_loader_spu_bases + 0x18)($at)
    lui $at, %hi(audio_loader_header_pointers + 0x10)
    sw $a0, %lo(audio_loader_header_pointers + 0x10)($at)
    lui $at, %hi(audio_loader_header_pointers + 0x2c)
    sw $t0, %lo(audio_loader_header_pointers + 0x2c)($at)
    lui $at, %hi(audio_loader_header_pointers + 0x14)
    sw $a2, %lo(audio_loader_header_pointers + 0x14)($at)
    lui $at, %hi(audio_loader_header_pointers + 0x30)
    sw $a3, %lo(audio_loader_header_pointers + 0x30)($at)
    lui $at, %hi(audio_loader_header_pointers + 0x18)
    sw $v0, %lo(audio_loader_header_pointers + 0x18)($at)
    lui $v0, %hi(audio_loader_auxiliary + 0x2e8)
    addiu $v0, $v0, %lo(audio_loader_auxiliary + 0x2e8)
    lui $at, %hi(audio_loader_header_pointers + 0x34)
    sw $v1, %lo(audio_loader_header_pointers + 0x34)($at)
    lui $at, %hi(audio_loader_slots + 0x78)
    sw $a1, %lo(audio_loader_slots + 0x78)($at)
    lui $at, %hi(audio_loader_slots + 0x80)
    sw $v0, %lo(audio_loader_slots + 0x80)($at)
    jr $ra
    nop
.end audio_loader_initialize_layout

.section .data
.balign 4
# exe/slus_004_22@0x80182328..0x80182348, live gate and seven callbacks.
# Unchecked callback indices beyond six reach the following SPU-base table.
.globl audio_cue_dispatch_prefix
audio_cue_dispatch_prefix:
    .word 0
    .word audio_cue_handler_0
    .word audio_cue_handler_1
    .word audio_cue_handler_2
    .word audio_cue_handler_3
    .word audio_cue_handler_4
    .word audio_cue_handler_5
    .word audio_cue_handler_6

# exe/slus_004_22@0x80182348..0x80182444, three mutable tables.
.globl audio_loader_spu_bases
audio_loader_spu_bases:
    .word 0x00001010, 0x0003e0a0, 0x0004f210, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    .word 0x00001010, 0x0003e0a0, 0x000520f0, 0x0005b170, 0x00062a90, 0x0006a3a0, 0x00071cc0
    .word 0x00001010, 0x0006c6d0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
.globl audio_loader_header_capacities
audio_loader_header_capacities:
    .word 0x00002a20, 0x00001020, 0x00001220, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    .word 0x00002620, 0x00000e20, 0x00000c20, 0x00001020, 0x00001020, 0x00001020, 0x00001a20
    .word 0x00003020, 0x00001220, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
.globl audio_loader_sequence_capacities
audio_loader_sequence_capacities:
    .word 0x0000c3a0, 0x00000000, 0x00001000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    .word 0x00008d60, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    .word 0x0000d3a0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000

.section .bss,"aw",@nobits
.balign 4
# exe/slus_004_22@0x8014677C, 140 bytes.
.globl audio_loader_slots
audio_loader_slots:
    .space 140

.section .bss,"aw",@nobits
.balign 4
# exe/slus_004_22@0x8011A000, 73728 bytes.
.globl audio_loader_staging
audio_loader_staging:
    .space 73728

.section .bss.audio_sound_auxiliary,"aw",@nobits
.balign 4
# exe/slus_004_22@0x801486A0, 868 bytes.
.globl audio_loader_auxiliary
audio_loader_auxiliary:
    .space 868

.section .bss.audio_sound_pointers,"aw",@nobits
.balign 4
# exe/slus_004_22@0x80148A18..0x80148A50, two seven-pointer arrays.
.globl audio_loader_header_pointers
audio_loader_header_pointers:
    .space 28
.globl audio_loader_sequence_pointers
audio_loader_sequence_pointers:
    .space 28
