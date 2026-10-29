# SPDX-License-Identifier: MIT
# Owned explicit-voice effects key-on and key-off.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Bank, voice and callback bindings remain live shared state.
.set noreorder
.set noat

.section .text
.balign 4
# exe/slus_004_22@0x8016E400..0x8016E794
.globl audio_effects_key_on
.ent audio_effects_key_on
audio_effects_key_on:
    addiu $sp, $sp, -0x40
    sw $s0, 0x18($sp)
    lw $s0, 0x58($sp)
    sw $s1, 0x1c($sp)
    lw $s1, 0x5c($sp)
    sw $s5, 0x2c($sp)
    lhu $s5, 0x50($sp)
    lui $v0, %hi(audio_sequence_voice_globals + 0x6f8)
    lw $v0, %lo(audio_sequence_voice_globals + 0x6f8)($v0)
    sw $s3, 0x24($sp)
    addu $s3, $a0, $zero
    sw $s7, 0x34($sp)
    addu $s7, $a2, $zero
    sw $s4, 0x28($sp)
    addu $s4, $a3, $zero
    sw $fp, 0x38($sp)
    lhu $fp, 0x54($sp)
    addiu $v1, $zero, 1
    sw $ra, 0x3c($sp)
    sw $s6, 0x30($sp)
    sw $s2, 0x20($sp)
    sh $a1, 0x10($sp)
    addu $s2, $s0, $zero
    bne $v0, $v1, .Leffects_8016e46c
    addu $s6, $s1, $zero
    j .Leffects_8016e760
    addiu $v0, $zero, -1
.Leffects_8016e46c:
    lui $at, %hi(audio_sequence_voice_globals + 0x6f8)
    sw $v1, %lo(audio_sequence_voice_globals + 0x6f8)($at)
    andi $v0, $a0, 0xffff
    sltiu $v0, $v0, 0x18
    beqz $v0, .Leffects_8016e650
    sll $a0, $a1, 0x10
    sra $a0, $a0, 0x10
    sll $a1, $a2, 0x10
    jal audio_sequence_select_bank_program
    sra $a1, $a1, 0x10
    bnez $v0, .Leffects_8016e650
    addiu $v0, $zero, 0x21
    lui $at, %hi(audio_sequence_allocation_mask + 0x1e)
    sh $v0, %lo(audio_sequence_allocation_mask + 0x1e)($at)
    sll $v0, $s0, 0x10
    sra $v1, $v0, 0x10
    sll $v0, $s1, 0x10
    sra $a0, $v0, 0x10
    lui $at, %hi(audio_sequence_allocation_mask + 0xa)
    sb $s5, %lo(audio_sequence_allocation_mask + 0xa)($at)
    lui $at, %hi(audio_sequence_allocation_mask + 0xb)
    sb $fp, %lo(audio_sequence_allocation_mask + 0xb)($at)
    lui $at, %hi(audio_sequence_allocation_mask + 0x14)
    sb $s4, %lo(audio_sequence_allocation_mask + 0x14)($at)
    bne $v1, $a0, .Leffects_8016e4f0
    slt $v0, $a0, $v1
    addiu $v0, $zero, 0x40
    lui $at, %hi(audio_sequence_allocation_mask + 0xd)
    sb $v0, %lo(audio_sequence_allocation_mask + 0xd)($at)
    lui $at, %hi(audio_sequence_allocation_mask + 0xc)
    sb $s2, %lo(audio_sequence_allocation_mask + 0xc)($at)
    j .Leffects_8016e580
    sll $v0, $s7, 0x10
.Leffects_8016e4f0:
    beqz $v0, .Leffects_8016e534
    sll $v0, $a0, 6
    div $zero, $v0, $v1
    bnez $v1, .Leffects_8016e508
    nop
    break 7
.Leffects_8016e508:
    addiu $at, $zero, -1
    bne $v1, $at, .Leffects_8016e520
    lui $at, 0x8000
    bne $v0, $at, .Leffects_8016e520
    nop
    break 6
.Leffects_8016e520:
    mflo $v0
    lui $at, %hi(audio_sequence_allocation_mask + 0xc)
    sb $s2, %lo(audio_sequence_allocation_mask + 0xc)($at)
    j .Leffects_8016e574
    nop
.Leffects_8016e534:
    sll $v1, $v1, 6
    div $zero, $v1, $a0
    bnez $a0, .Leffects_8016e548
    nop
    break 7
.Leffects_8016e548:
    addiu $at, $zero, -1
    bne $a0, $at, .Leffects_8016e560
    lui $at, 0x8000
    bne $v1, $at, .Leffects_8016e560
    nop
    break 6
.Leffects_8016e560:
    mflo $v1
    addiu $v0, $zero, 0x7f
    lui $at, %hi(audio_sequence_allocation_mask + 0xc)
    sb $s6, %lo(audio_sequence_allocation_mask + 0xc)($at)
    subu $v0, $v0, $v1
.Leffects_8016e574:
    lui $at, %hi(audio_sequence_allocation_mask + 0xd)
    sb $v0, %lo(audio_sequence_allocation_mask + 0xd)($at)
    sll $v0, $s7, 0x10
.Leffects_8016e580:
    lui $v1, %hi(audio_sequence_voice_globals + 0x700)
    lw $v1, %lo(audio_sequence_voice_globals + 0x700)($v1)
    sra $v0, $v0, 0xc
    addu $v0, $v0, $v1
    lbu $v1, 1($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x12)
    sb $v1, %lo(audio_sequence_allocation_mask + 0x12)($at)
    lbu $v1, 4($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x13)
    sb $v1, %lo(audio_sequence_allocation_mask + 0x13)($at)
    lbu $v0, 0($v0)
    lui $v1, %hi(audio_sequence_allocation_mask + 0xf)
    lbu $v1, %lo(audio_sequence_allocation_mask + 0xf)($v1)
    lui $at, %hi(audio_sequence_allocation_mask + 0x8)
    sb $v0, %lo(audio_sequence_allocation_mask + 0x8)($at)
    lui $v0, %hi(audio_sequence_allocation_mask + 0x14)
    lbu $v0, %lo(audio_sequence_allocation_mask + 0x14)($v0)
    sll $v1, $v1, 4
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_sequence_voice_globals + 0x70c)
    lw $v1, %lo(audio_sequence_voice_globals + 0x70c)($v1)
    sll $v0, $v0, 5
    addu $v0, $v0, $v1
.globl audio_effects_tone_read
audio_effects_tone_read:
    lbu $v1, 0($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x17)
    sb $v1, %lo(audio_sequence_allocation_mask + 0x17)($at)
    lhu $a0, 0x16($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x20)
    sh $a0, %lo(audio_sequence_allocation_mask + 0x20)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x15)
    sb $v1, %lo(audio_sequence_allocation_mask + 0x15)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x16)
    sb $v1, %lo(audio_sequence_allocation_mask + 0x16)($at)
    lbu $v1, 4($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x18)
    sb $v1, %lo(audio_sequence_allocation_mask + 0x18)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x19)
    sb $v1, %lo(audio_sequence_allocation_mask + 0x19)($at)
    lbu $v1, 1($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x1c)
    sb $v1, %lo(audio_sequence_allocation_mask + 0x1c)($at)
    lbu $v1, 6($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x1a)
    sb $v1, %lo(audio_sequence_allocation_mask + 0x1a)($at)
    lbu $v0, 7($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 0x1b)
    sb $v0, %lo(audio_sequence_allocation_mask + 0x1b)($at)
    bnez $a0, .Leffects_8016e660
    sll $v1, $s3, 0x10
.Leffects_8016e650:
    lui $at, %hi(audio_sequence_voice_globals + 0x6f8)
    sw $zero, %lo(audio_sequence_voice_globals + 0x6f8)($at)
    j .Leffects_8016e760
    addiu $v0, $zero, -1
.Leffects_8016e660:
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addiu $v1, $zero, 0x21
    lui $at, %hi(audio_sequence_active_voice)
    sh $s3, %lo(audio_sequence_active_voice)($at)
    lui $at, %hi(audio_sequence_voice_globals + 0xae)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 0xae)($at)
    lhu $t0, 0x10($sp)
    lui $at, %hi(audio_sequence_voice_globals + 0xb6)
    addu $at, $at, $v0
    sh $t0, %lo(audio_sequence_voice_globals + 0xb6)($at)
    lui $v1, %hi(audio_sequence_allocation_mask + 0xf)
    lbu $v1, %lo(audio_sequence_allocation_mask + 0xf)($v1)
    lui $at, %hi(audio_sequence_voice_globals + 0xb2)
    addu $at, $at, $v0
    sh $s7, %lo(audio_sequence_voice_globals + 0xb2)($at)
    lui $at, %hi(audio_sequence_voice_globals + 0xb0)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 0xb0)($at)
    lui $v1, %hi(audio_sequence_allocation_mask + 0x20)
    lhu $v1, %lo(audio_sequence_allocation_mask + 0x20)($v1)
    lui $at, %hi(audio_sequence_voice_globals + 0xa0)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 0xa0)($at)
    lui $a0, %hi(audio_sequence_allocation_mask + 0x14)
    lbu $a0, %lo(audio_sequence_allocation_mask + 0x14)($a0)
    addiu $v1, $zero, 1
    lui $at, %hi(audio_sequence_voice_globals + 0xac)
    addu $at, $at, $v0
    sh $s5, %lo(audio_sequence_voice_globals + 0xac)($at)
    lui $at, %hi(audio_sequence_voice_globals + 0xbb)
    addu $at, $at, $v0
    sb $v1, %lo(audio_sequence_voice_globals + 0xbb)($at)
    lui $at, %hi(audio_sequence_voice_globals + 0xa2)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 0xa2)($at)
    lui $at, %hi(audio_sequence_voice_globals + 0xb4)
    addu $at, $at, $v0
    sh $a0, %lo(audio_sequence_voice_globals + 0xb4)($at)
    jal audio_sequence_prepare_voice
    nop
    lui $v1, %hi(audio_sequence_allocation_mask + 0x20)
    lh $v1, %lo(audio_sequence_allocation_mask + 0x20)($v1)
    addiu $v0, $zero, 0xff
    bne $v1, $v0, .Leffects_8016e73c
    addu $a0, $s5, $zero
    jal audio_sequence_key_noise
    andi $a0, $s3, 0xff
    j .Leffects_8016e750
    nop
.Leffects_8016e73c:
    jal audio_sequence_note_to_pitch
    addu $a1, $fp, $zero
    addiu $a0, $zero, 1
    jal audio_sequence_key_voice
    andi $a1, $v0, 0xffff
.Leffects_8016e750:
    lui $at, %hi(audio_sequence_voice_globals + 0x6f8)
    sw $zero, %lo(audio_sequence_voice_globals + 0x6f8)($at)
    sll $v0, $s3, 0x10
    sra $v0, $v0, 0x10
.Leffects_8016e760:
    lw $ra, 0x3c($sp)
    lw $fp, 0x38($sp)
    lw $s7, 0x34($sp)
    lw $s6, 0x30($sp)
    lw $s5, 0x2c($sp)
    lw $s4, 0x28($sp)
    lw $s3, 0x24($sp)
    lw $s2, 0x20($sp)
    lw $s1, 0x1c($sp)
    lw $s0, 0x18($sp)
    addiu $sp, $sp, 0x40
    jr $ra
    nop
.end audio_effects_key_on

.section .text
.balign 4
# exe/slus_004_22@0x8016E794..0x8016E80C
.globl audio_effects_key_off
.ent audio_effects_key_off
audio_effects_key_off:
    addiu $sp, $sp, -0x18
    addu $a1, $a0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 0x6f8)
    lw $v0, %lo(audio_sequence_voice_globals + 0x6f8)($v0)
    addiu $v1, $zero, 1
    bne $v0, $v1, .Leffects_8016e7b8
    sw $ra, 0x10($sp)
    j .Leffects_8016e7fc
    addiu $v0, $zero, -1
.Leffects_8016e7b8:
    lui $at, %hi(audio_sequence_voice_globals + 0x6f8)
    sw $v1, %lo(audio_sequence_voice_globals + 0x6f8)($at)
    andi $v0, $a0, 0xffff
    sltiu $v0, $v0, 0x18
    bnez $v0, .Leffects_8016e7e0
    addiu $v0, $zero, -1
    lui $at, %hi(audio_sequence_voice_globals + 0x6f8)
    sw $zero, %lo(audio_sequence_voice_globals + 0x6f8)($at)
    j .Leffects_8016e7fc
    nop
.Leffects_8016e7e0:
    lui $at, %hi(audio_sequence_active_voice)
    sh $a1, %lo(audio_sequence_active_voice)($at)
    jal audio_sequence_release_voice
    addu $a0, $zero, $zero
    addu $v0, $zero, $zero
    lui $at, %hi(audio_sequence_voice_globals + 0x6f8)
    sw $zero, %lo(audio_sequence_voice_globals + 0x6f8)($at)
.Leffects_8016e7fc:
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_effects_key_off
