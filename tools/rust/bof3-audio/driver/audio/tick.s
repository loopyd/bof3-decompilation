# SPDX-License-Identifier: MIT
# Owned sequence tick-mode initialization and live video-mode input.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# All six dispatch cases, signed fallback and bit-0x1000 behavior remain.
# The caller supplies audio_video_mode before initialization.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x8016D9CC..0x8016DB38
.globl audio_sequence_initialize_tick
.ent audio_sequence_initialize_tick
audio_sequence_initialize_tick:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    sw $ra, 0x14($sp)
    jal audio_video_get_mode
    addu $s0, $a0, $zero
    addu $a0, $v0, $zero
    andi $v0, $s0, 0x1000
    beqz $v0, .Ltick_8016da0c
    addiu $v0, $zero, 1
    lui $at, %hi(audio_sequence_tick_flag)
    sw $v0, %lo(audio_sequence_tick_flag)($at)
    andi $v0, $s0, 0xfff
    lui $at, %hi(audio_sequence_tick_mode)
    sw $v0, %lo(audio_sequence_tick_mode)($at)
    j .Ltick_8016da1c
    nop
.Ltick_8016da0c:
    lui $at, %hi(audio_sequence_tick_flag)
    sw $zero, %lo(audio_sequence_tick_flag)($at)
    lui $at, %hi(audio_sequence_tick_mode)
    sw $s0, %lo(audio_sequence_tick_mode)($at)
.Ltick_8016da1c:
    lui $v1, %hi(audio_sequence_tick_mode)
    lw $v1, %lo(audio_sequence_tick_mode)($v1)
    nop
    slti $v0, $v1, 6
    beqz $v0, .Ltick_8016db1c
    sltiu $v0, $v1, 6
    beqz $v0, .Ltick_8016db08
    sll $v0, $v1, 2
    lui $at, %hi(audio_sequence_tick_dispatch)
    addu $at, $at, $v0
    lw $v0, %lo(audio_sequence_tick_dispatch)($at)
    nop
    jr $v0
    nop
.Ltick_8016da54:
    addiu $v1, $zero, 0x32
    lui $at, %hi(audio_sequence_tick_rate)
    sw $v1, %lo(audio_sequence_tick_rate)($at)
    addiu $v0, $zero, 1
    beq $a0, $v0, .Ltick_8016da94
    addiu $v0, $zero, 5
    lui $at, %hi(audio_sequence_tick_mode)
    sw $v1, %lo(audio_sequence_tick_mode)($at)
    j .Ltick_8016db24
    nop
.Ltick_8016da7c:
    addiu $v0, $zero, 0x3c
    lui $at, %hi(audio_sequence_tick_rate)
    sw $v0, %lo(audio_sequence_tick_rate)($at)
    bnez $a0, .Ltick_8016da94
    nop
    addiu $v0, $zero, 5
.Ltick_8016da94:
    lui $at, %hi(audio_sequence_tick_mode)
    sw $v0, %lo(audio_sequence_tick_mode)($at)
    j .Ltick_8016db24
    nop
.Ltick_8016daa4:
    addiu $v0, $zero, 0x78
    lui $at, %hi(audio_sequence_tick_rate)
    sw $v0, %lo(audio_sequence_tick_rate)($at)
    j .Ltick_8016db24
    nop
.Ltick_8016dab8:
    addiu $v0, $zero, 0xf0
    lui $at, %hi(audio_sequence_tick_rate)
    sw $v0, %lo(audio_sequence_tick_rate)($at)
    j .Ltick_8016db24
    nop
.Ltick_8016dacc:
    beqz $a0, .Ltick_8016db08
    addiu $v0, $zero, 1
    beq $a0, $v0, .Ltick_8016daf8
    addiu $v0, $zero, 0x32
    j .Ltick_8016db0c
    addiu $v0, $zero, 0x3c
.Ltick_8016dae4:
    beqz $a0, .Ltick_8016db08
    addiu $v0, $zero, 1
    bne $a0, $v0, .Ltick_8016db0c
    addiu $v0, $zero, 0x3c
    addiu $v0, $zero, 0x32
.Ltick_8016daf8:
    lui $at, %hi(audio_sequence_tick_rate)
    sw $v0, %lo(audio_sequence_tick_rate)($at)
    j .Ltick_8016db24
    nop
.Ltick_8016db08:
    addiu $v0, $zero, 0x3c
.Ltick_8016db0c:
    lui $at, %hi(audio_sequence_tick_rate)
    sw $v0, %lo(audio_sequence_tick_rate)($at)
    j .Ltick_8016db24
    nop
.Ltick_8016db1c:
    lui $at, %hi(audio_sequence_tick_rate)
    sw $v1, %lo(audio_sequence_tick_rate)($at)
.Ltick_8016db24:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_initialize_tick

# exe/slus_004_22@0x801753DC..0x801753EC
.globl audio_video_get_mode
.ent audio_video_get_mode
audio_video_get_mode:
    lui $v0, %hi(audio_video_mode)
    lw $v0, %lo(audio_video_mode)($v0)
    jr $ra
    nop
.end audio_video_get_mode

.section .data
.balign 4
# exe/slus_004_22@0x8014A0C4, six bounded dispatch entries.
.globl audio_sequence_tick_dispatch
audio_sequence_tick_dispatch:
    .word .Ltick_8016dae4
    .word .Ltick_8016da7c
    .word .Ltick_8016dab8
    .word .Ltick_8016daa4
    .word .Ltick_8016da54
    .word .Ltick_8016dacc

.section .data
# exe/slus_004_22@0x80184440, one accessed word.
.balign 4
.globl audio_sequence_tick_mode
audio_sequence_tick_mode:
    .word 60
# exe/slus_004_22@0x80184444, one accessed word.
.balign 4
.globl audio_sequence_tick_flag
audio_sequence_tick_flag:
    .word 1

.section .bss
# exe/slus_004_22@0x80190300, one accessed word.
.balign 4
.globl audio_sequence_tick_rate
audio_sequence_tick_rate:
    .space 4
# exe/slus_004_22@0x801856F4, one accessed word.
.balign 4
.globl audio_video_mode
audio_video_mode:
    .space 4
