# SPDX-License-Identifier: MIT
# Owned XA selector with caller-bound, live cue tables and indexed file LBAs.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
.set noreorder
.set noat

.section .text
.balign 4
# exe/slus_004_22@0x80163744..0x80163858.
# A0 >> 12 selects one of three streams. The low 12 bits select a cue.
# The caller must bind the complete table walk and indexed LBA before entry.
.globl audio_xa_select_cue
.ent audio_xa_select_cue
audio_xa_select_cue:
    addiu $sp, $sp, -8
    sra $v0, $a0, 12
    beqz $v0, .Lxa_80163758
    addiu $t0, $zero, 8
    addiu $t0, $zero, 16
.Lxa_80163758:
    lui $a1, %hi(audio_xa_state + 0x29)
    addiu $a1, $a1, %lo(audio_xa_state + 0x29)
    sll $v0, $v0, 2
    sb $zero, 0($a1)
    lui $at, %hi(audio_xa_table_pointers)
    addu $at, $at, $v0
    lw $a3, %lo(audio_xa_table_pointers)($at)
    andi $v0, $a0, 0xfff
    blez $v0, .Lxa_801637b8
    addu $v1, $zero, $zero
    addu $a2, $v0, $zero
.Lxa_80163784:
    lhu $v0, 0($a3)
    nop
    andi $v0, $v0, 0x8000
    beqz $v0, .Lxa_801637a8
    nop
    lbu $v0, 0($a1)
    addiu $a3, $a3, 2
    addiu $v0, $v0, 1
    sb $v0, 0($a1)
.Lxa_801637a8:
    addiu $v1, $v1, 1
    slt $v0, $v1, $a2
    bnez $v0, .Lxa_80163784
    addiu $a3, $a3, 2
.Lxa_801637b8:
    lui $a2, %hi(audio_xa_state + 0x28)
    addiu $a2, $a2, %lo(audio_xa_state + 0x28)
    addiu $v0, $zero, 1
    sb $v0, 0($a2)
    sra $v0, $a0, 12
    sll $v0, $v0, 2
    lui $at, %hi(audio_xa_file_slots)
    addu $at, $at, $v0
    lw $v0, %lo(audio_xa_file_slots)($at)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_xa_file_lbas)
    addu $at, $at, $v0
    lw $a0, %lo(audio_xa_file_lbas)($at)
    lui $at, %hi(audio_xa_state + 4)
    sw $a0, %lo(audio_xa_state + 4)($at)
    sw $a0, -0x28($a2)
    lhu $v0, 0($a3)
    nop
    andi $v0, $v0, 0x7fff
    mult $v0, $t0
    lbu $a1, 1($a2)
    mflo $t1
    addu $v1, $t1, $a1
    addu $v1, $a0, $v1
    sw $v1, -0x28($a2)
    lhu $v0, 2($a3)
    nop
    andi $v0, $v0, 0x7fff
    mult $v0, $t0
    addiu $a0, $a0, -0x96
    sw $v1, -0x20($a2)
    mflo $t1
    addu $a1, $t1, $a1
    addu $a0, $a0, $a1
    lui $at, %hi(audio_xa_state + 4)
    sw $a0, %lo(audio_xa_state + 4)($at)
    addiu $sp, $sp, 8
    jr $ra
    nop
.end audio_xa_select_cue

.section .data
.balign 4
# Original adjacent arrays at 0x80183230..0x80183248.
.globl audio_xa_file_slots
audio_xa_file_slots:
    .word 681, 434, 682
.globl audio_xa_table_pointers
audio_xa_table_pointers:
    .word 0, 0, 0

.section .bss
.balign 4
# Original zero state at 0x8014680C..0x80146841, rounded to word alignment.
.globl audio_xa_state
audio_xa_state:
    .space 56

.balign 4
# Caller-bound indexed storage corresponding to 0x80182444 slots 0..682.
# Only explicitly supplied entries are configured; zero is not a media binding.
.globl audio_xa_file_lbas
audio_xa_file_lbas:
    .space 2732
