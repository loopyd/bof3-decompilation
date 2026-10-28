# SPDX-License-Identifier: MIT
# Owned sequence configuration and startup controls.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Counts remain unchecked signed halfwords. Callers own record storage.
# Retain live pointer/count reloads and selective record writes.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x8016D7EC..0x8016D9CC
.globl audio_sequence_configure
.ent audio_sequence_configure
audio_sequence_configure:
    addiu $sp, $sp, -0x18
    lui $at, %hi(audio_sequence_handle_count)
    sh $a1, %lo(audio_sequence_handle_count)($at)
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
    lui $at, %hi(audio_sequence_count)
    sh $a2, %lo(audio_sequence_count)($at)
    blez $a1, .Lconfiguration_8016d85c
    addu $a3, $zero, $zero
    sll $v0, $a2, 0x10
    sra $t0, $v0, 0x10
    lui $a2, %hi(audio_sequence_handles)
    addiu $a2, $a2, %lo(audio_sequence_handles)
    addu $v1, $zero, $zero
.Lconfiguration_8016d824:
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $a0, $v0
    sw $v0, 0($a2)
    addiu $a2, $a2, 4
    addiu $a3, $a3, 1
    slt $v0, $a3, $a1
    bnez $v0, .Lconfiguration_8016d824
    addu $v1, $v1, $t0
.Lconfiguration_8016d85c:
    lui $v0, %hi(audio_sequence_handle_count)
    lh $v0, %lo(audio_sequence_handle_count)($v0)
    nop
    blez $v0, .Lconfiguration_8016d9c0
    addu $a3, $zero, $zero
    addiu $t0, $zero, 0x7f
    lui $t1, %hi(audio_sequence_handles)
    addiu $t1, $t1, %lo(audio_sequence_handles)
.Lconfiguration_8016d87c:
    lui $v0, %hi(audio_sequence_count)
    lh $v0, %lo(audio_sequence_count)($v0)
    nop
    blez $v0, .Lconfiguration_8016d9a8
    addu $a2, $zero, $zero
    addu $a1, $t1, $zero
    addu $a0, $zero, $zero
.Lconfiguration_8016d898:
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sw $zero, 0x90($v0)
    lw $v0, 0($a1)
    addiu $v1, $zero, 0xff
    addu $v0, $a0, $v0
    sb $v1, 0x3c($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sb $zero, 0($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sh $zero, 0x3e($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sh $zero, 0x40($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sw $zero, 0x94($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sw $zero, 0x98($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sh $zero, 0x42($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sw $zero, 0xa4($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sw $zero, 0xa0($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sw $zero, 0x9c($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sh $zero, 0x44($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sh $t0, 0x74($v0)
    lw $v0, 0($a1)
    nop
    addu $v0, $a0, $v0
    sh $t0, 0x76($v0)
    lw $v0, 0($a1)
    addiu $a2, $a2, 1
    addu $v0, $a0, $v0
    sh $t0, 0x78($v0)
    lw $v1, 0($a1)
    lui $v0, %hi(audio_sequence_count)
    lh $v0, %lo(audio_sequence_count)($v0)
    addu $v1, $a0, $v1
    addiu $a0, $a0, 0xac
    slt $v0, $a2, $v0
    bnez $v0, .Lconfiguration_8016d898
    sh $t0, 0x7a($v1)
.Lconfiguration_8016d9a8:
    lui $v0, %hi(audio_sequence_handle_count)
    lh $v0, %lo(audio_sequence_handle_count)($v0)
    addiu $a3, $a3, 1
    slt $v0, $a3, $v0
    bnez $v0, .Lconfiguration_8016d87c
    addiu $t1, $t1, 4
.Lconfiguration_8016d9c0:
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_configure

# exe/slus_004_22@0x801737B4..0x801737C4
.globl audio_sequence_set_playback_byte
.ent audio_sequence_set_playback_byte
audio_sequence_set_playback_byte:
    lui $at, %hi(audio_sequence_playback_byte)
    sb $a0, %lo(audio_sequence_playback_byte)($at)
    jr $ra
    nop
.end audio_sequence_set_playback_byte

# exe/slus_004_22@0x801737E8..0x80173818
.globl audio_sequence_set_voice_count
.ent audio_sequence_set_voice_count
audio_sequence_set_voice_count:
    andi $v1, $a0, 0xff
    sltiu $v0, $v1, 0x19
    beqz $v0, .Lconfiguration_80173800
    nop
    bnez $v1, .Lconfiguration_80173808
    addu $v0, $v1, $zero
.Lconfiguration_80173800:
    j .Lconfiguration_80173810
    addiu $v0, $zero, 0xff
.Lconfiguration_80173808:
    lui $at, %hi(audio_sequence_voice_count)
    sb $a0, %lo(audio_sequence_voice_count)($at)
.Lconfiguration_80173810:
    jr $ra
    nop
.end audio_sequence_set_voice_count

.section .bss
.balign 4
# Keep the 32 handle pointers, 32-by-16-word matrix and counts adjacent.
# The allocator uses a 32-bit handle mask. Configuration adds no bounds check.
# exe/slus_004_22@0x80190308, 128 bytes.
.globl audio_sequence_handles
audio_sequence_handles:
    .space 128
# exe/slus_004_22@0x80190388, 2048 bytes.
.globl audio_sequence_channel_matrix
audio_sequence_channel_matrix:
    .space 2048
# exe/slus_004_22@0x80190B88, 2 bytes.
.globl audio_sequence_handle_count
audio_sequence_handle_count:
    .space 2
# exe/slus_004_22@0x80190B8A, 2 bytes.
.globl audio_sequence_count
audio_sequence_count:
    .space 2
# exe/slus_004_22@0x8018E98C, 1 byte.
.globl audio_sequence_playback_byte
audio_sequence_playback_byte:
    .space 1
# exe/slus_004_22@0x8018E264, 1 byte.
.globl audio_sequence_voice_count
audio_sequence_voice_count:
    .space 1
