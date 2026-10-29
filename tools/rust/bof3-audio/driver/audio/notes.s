# SPDX-License-Identifier: MIT
# Owned sequence note dispatch, allocation, voice preparation and key control.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Preserve full original bodies, live shared selection and ordered register access.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x8016A974..0x8016AA5C
.globl audio_sequence_note_command
.ent audio_sequence_note_command
audio_sequence_note_command:
    addiu $sp, $sp, -0x20
    addu $t0, $a0, $zero
    addu $t3, $a2, $zero
    sll $a0, $t0, 0x10
    sra $a0, $a0, 0xe
    sll $v1, $a1, 0x10
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    sw $ra, 0x18($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a0
    lw $v1, %lo(audio_sequence_handles)($at)
    nop
    addu $v1, $v1, $v0
    lbu $a0, 0x12($v1)
    lhu $v0, 0x74($v1)
    addu $a2, $v1, $a0
    lbu $t1, 0x17($a2)
    beqz $v0, .Lnotes_8016aa4c
    andi $t2, $a3, 0xff
    andi $v0, $a3, 0xff
    beqz $v0, .Lnotes_8016aa2c
    nop
    lh $v0, 0xa8($v1)
    nop
    srav $v0, $v0, $a0
    andi $v0, $v0, 1
    bnez $v0, .Lnotes_8016aa4c
    sll $a0, $a1, 8
    or $a0, $t0, $a0
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    lh $a1, 0x4c($v1)
    lbu $a2, 0x2c($a2)
    andi $a3, $t3, 0xff
    sw $t2, 0x10($sp)
    jal audio_sequence_note_on
    sw $t1, 0x14($sp)
    j .Lnotes_8016aa4c
    nop
.Lnotes_8016aa2c:
    sll $a0, $a1, 8
    or $a0, $t0, $a0
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    lh $a1, 0x4c($v1)
    lbu $a2, 0x2c($a2)
    jal audio_sequence_note_off
    andi $a3, $t3, 0xff
.Lnotes_8016aa4c:
    lw $ra, 0x18($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_note_command

# exe/slus_004_22@0x8016FA7C..0x8016FC90
.globl audio_sequence_prepare_voice
.ent audio_sequence_prepare_voice
audio_sequence_prepare_voice:
    addiu $sp, $sp, -8
    addu $a2, $zero, $zero
    lui $v1, %hi(audio_sequence_selected_register_index)
    addiu $v1, $v1, %lo(audio_sequence_selected_register_index)
    addiu $t0, $v1, -2
    addiu $a3, $zero, 1
    lui $a0, %hi(audio_sequence_active_voice)
    lh $a0, %lo(audio_sequence_active_voice)($a0)
    lui $a1, %hi(audio_sequence_envelope_history)
    addiu $a1, $a1, %lo(audio_sequence_envelope_history)
    sll $v0, $a0, 3
    sh $v0, 0($v1)
    lui $v0, %hi(audio_sequence_selected_tone_block)
    lbu $v0, %lo(audio_sequence_selected_tone_block)($v0)
    lui $v1, %hi(audio_sequence_allocation_mask + 20)
    lbu $v1, %lo(audio_sequence_allocation_mask + 20)($v1)
    sll $v0, $v0, 4
    addu $v1, $v1, $v0
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_selected_tone_index)
    sh $v1, %lo(audio_sequence_selected_tone_index)($at)
    addiu $v1, $zero, 0x7fff
    lui $at, %hi(audio_sequence_voice_globals + 166)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 166)($at)
.Lnotes_8016faf0:
    addiu $a2, $a2, 1
    lh $v0, 0($t0)
    lw $v1, 0($a1)
    sllv $v0, $a3, $v0
    nor $v0, $zero, $v0
    and $v1, $v1, $v0
    sw $v1, 0($a1)
    slti $v0, $a2, 0x10
    bnez $v0, .Lnotes_8016faf0
    addiu $a1, $a1, 4
    lui $v1, %hi(audio_sequence_allocation_mask + 32)
    lhu $v1, %lo(audio_sequence_allocation_mask + 32)($v1)
    nop
    andi $v0, $v1, 1
    blez $v0, .Lnotes_8016fb68
    sll $v0, $v1, 0x10
    sra $v0, $v0, 0x10
    addiu $v0, $v0, -1
    srl $v1, $v0, 0x1f
    addu $v0, $v0, $v1
    sra $v0, $v0, 1
    lui $v1, %hi(audio_sequence_selected_programs)
    lw $v1, %lo(audio_sequence_selected_programs)($v1)
    sll $v0, $v0, 4
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_sequence_selected_register_index)
    lh $v1, %lo(audio_sequence_selected_register_index)($v1)
    lhu $v0, 0xc($v0)
    j .Lnotes_8016fb9c
    sll $v1, $v1, 1
.Lnotes_8016fb68:
    sra $v0, $v0, 0x10
    addiu $v0, $v0, -1
    srl $v1, $v0, 0x1f
    addu $v0, $v0, $v1
    sra $v0, $v0, 1
    lui $v1, %hi(audio_sequence_selected_programs)
    lw $v1, %lo(audio_sequence_selected_programs)($v1)
    sll $v0, $v0, 4
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_sequence_selected_register_index)
    lh $v1, %lo(audio_sequence_selected_register_index)($v1)
    lhu $v0, 0xe($v0)
    sll $v1, $v1, 1
.Lnotes_8016fb9c:
    lui $at, %hi(audio_sequence_voice_flags + 22)
    addu $at, $at, $v1
    sh $v0, %lo(audio_sequence_voice_flags + 22)($at)
    lui $v1, %hi(audio_sequence_active_voice)
    lh $v1, %lo(audio_sequence_active_voice)($v1)
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $v1
    lbu $v0, %lo(audio_sequence_voice_dirty)($at)
    nop
    ori $v0, $v0, 8
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $v1
    sb $v0, %lo(audio_sequence_voice_dirty)($at)
    lui $a1, %hi(audio_sequence_selected_register_index)
    addiu $a1, $a1, %lo(audio_sequence_selected_register_index)
    lui $v0, %hi(audio_sequence_selected_tone_block)
    lbu $v0, %lo(audio_sequence_selected_tone_block)($v0)
    lui $v1, %hi(audio_sequence_allocation_mask + 20)
    lbu $v1, %lo(audio_sequence_allocation_mask + 20)($v1)
    lui $a0, %hi(audio_sequence_selected_tones)
    lw $a0, %lo(audio_sequence_selected_tones)($a0)
    sll $v0, $v0, 4
    addu $v0, $v0, $v1
    sll $v0, $v0, 5
    addu $v0, $v0, $a0
    lh $v1, 0($a1)
    lhu $v0, 0x10($v0)
    sll $v1, $v1, 1
    lui $at, %hi(audio_sequence_voice_flags + 24)
    addu $at, $at, $v1
    sh $v0, %lo(audio_sequence_voice_flags + 24)($at)
    lui $v0, %hi(audio_sequence_selected_tone_block)
    lbu $v0, %lo(audio_sequence_selected_tone_block)($v0)
    lui $v1, %hi(audio_sequence_allocation_mask + 20)
    lbu $v1, %lo(audio_sequence_allocation_mask + 20)($v1)
    sll $v0, $v0, 4
    addu $v0, $v0, $v1
    sll $v0, $v0, 5
    addu $v0, $v0, $a0
    lh $v1, 0($a1)
    lhu $v0, 0x12($v0)
    lui $a0, %hi(audio_sequence_voice_state_e1e0)
    lhu $a0, %lo(audio_sequence_voice_state_e1e0)($a0)
    sll $v1, $v1, 1
    addu $v0, $v0, $a0
    lui $at, %hi(audio_sequence_voice_flags + 26)
    addu $at, $at, $v1
    sh $v0, %lo(audio_sequence_voice_flags + 26)($at)
    lui $v1, %hi(audio_sequence_active_voice)
    lh $v1, %lo(audio_sequence_active_voice)($v1)
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $v1
    lbu $v0, %lo(audio_sequence_voice_dirty)($at)
    nop
    ori $v0, $v0, 0x30
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $v1
    sb $v0, %lo(audio_sequence_voice_dirty)($at)
    addiu $sp, $sp, 8
    jr $ra
    nop
.end audio_sequence_prepare_voice

# exe/slus_004_22@0x8017102C..0x8017157C
.globl audio_sequence_note_on
.ent audio_sequence_note_on
audio_sequence_note_on:
    addiu $sp, $sp, -0x148
    addu $v1, $a0, $zero
    addu $v0, $a1, $zero
    sw $s3, 0x12c($sp)
    addu $s3, $zero, $zero
    sll $a0, $v0, 0x10
    sra $a0, $a0, 0x10
    sll $a1, $a2, 0x10
    sra $a1, $a1, 0x10
    sw $s5, 0x134($sp)
    addu $s5, $v1, $zero
    sw $s4, 0x130($sp)
    addu $s4, $v0, $zero
    sw $s7, 0x13c($sp)
    addu $s7, $a3, $zero
    sh $a2, 0x110($sp)
    andi $a2, $v1, 0xff
    sll $a2, $a2, 2
    sll $v1, $v1, 0x10
    sw $s1, 0x124($sp)
    sra $s1, $v1, 0x10
    srl $v1, $v1, 0x18
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    sw $ra, 0x144($sp)
    sw $fp, 0x140($sp)
    sw $s6, 0x138($sp)
    sw $s2, 0x128($sp)
    sw $s0, 0x120($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a2
    lw $v1, %lo(audio_sequence_handles)($at)
    lhu $s6, 0x158($sp)
    lhu $fp, 0x15c($sp)
    jal audio_sequence_select_bank_program
    addu $s0, $v1, $v0
    bnez $v0, .Lnotes_80171548
    addiu $v0, $zero, -1
    addiu $v0, $zero, 0x21
    lui $at, %hi(audio_sequence_voice_owner)
    sh $s5, %lo(audio_sequence_voice_owner)($at)
    lui $at, %hi(audio_sequence_allocation_mask + 10)
    sb $s7, %lo(audio_sequence_allocation_mask + 10)($at)
    lui $at, %hi(audio_sequence_allocation_mask + 11)
    sb $zero, %lo(audio_sequence_allocation_mask + 11)($at)
    bne $s1, $v0, .Lnotes_8017110c
    lui $v1, 0x8102
    lui $at, %hi(audio_sequence_selected_tone)
    sb $s6, %lo(audio_sequence_selected_tone)($at)
    j .Lnotes_80171154
    nop
.Lnotes_8017110c:
    lbu $v0, 0x12($s0)
    nop
    sll $v0, $v0, 1
    addu $v0, $v0, $s0
    lh $v0, 0x4e($v0)
    nop
    mult $s6, $v0
    mflo $v0
    ori $v1, $v1, 0x409
    nop
    mult $v0, $v1
    mfhi $t0
    addu $v1, $t0, $v0
    sra $v1, $v1, 6
    sra $v0, $v0, 0x1f
    subu $v1, $v1, $v0
    lui $at, %hi(audio_sequence_selected_tone)
    sb $v1, %lo(audio_sequence_selected_tone)($at)
.Lnotes_80171154:
    lui $s2, %hi(audio_sequence_allocation_mask + 13)
    addiu $s2, $s2, %lo(audio_sequence_allocation_mask + 13)
    sb $fp, 0($s2)
    lhu $t0, 0x110($sp)
    lui $v1, %hi(audio_sequence_selected_programs)
    lw $v1, %lo(audio_sequence_selected_programs)($v1)
    sll $v0, $t0, 0x10
    sra $a2, $v0, 0x10
    sll $v0, $a2, 4
    addu $v0, $v0, $v1
    lbu $v1, 1($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 18)
    sb $v1, %lo(audio_sequence_allocation_mask + 18)($at)
    lbu $v1, 4($v0)
    lui $at, %hi(audio_sequence_allocation_mask + 19)
    sb $v1, %lo(audio_sequence_allocation_mask + 19)($at)
    lbu $v0, 0($v0)
    lui $v1, %hi(audio_sequence_selected_header)
    lw $v1, %lo(audio_sequence_selected_header)($v1)
    lui $at, %hi(audio_sequence_tone_selection)
    sb $v0, %lo(audio_sequence_tone_selection)($at)
    lui $v0, %hi(audio_sequence_selected_tone_block)
    lbu $v0, %lo(audio_sequence_selected_tone_block)($v0)
    lhu $v1, 0x12($v1)
    nop
    sltu $v0, $v0, $v1
    beqz $v0, .Lnotes_80171548
    addiu $v0, $zero, -1
    bnez $s6, .Lnotes_801711ec
    addiu $a0, $sp, 0x90
    sll $a0, $s5, 0x10
    sra $a0, $a0, 0x10
    sll $a1, $s4, 0x10
    sra $a1, $a1, 0x10
    jal audio_sequence_note_off
    andi $a3, $s7, 0xffff
    j .Lnotes_80171544
    addu $s3, $v0, $zero
.Lnotes_801711ec:
    jal audio_sequence_select_tones
    addiu $a1, $sp, 0x10
    addu $s4, $v0, $zero
    andi $v0, $s4, 0xff
    beqz $v0, .Lnotes_80171544
    addu $s1, $zero, $zero
    addiu $s0, $s2, 0x13
    addiu $s2, $s2, 0x15
    andi $v1, $s1, 0xff
.Lnotes_80171210:
    addu $v0, $sp, $v1
    lbu $v0, 0x10($v0)
    nop
    sh $v0, 0($s0)
    addu $v0, $sp, $v1
    lbu $v1, 0x90($v0)
    lbu $v0, -0x11($s0)
    nop
    sll $v0, $v0, 4
    sb $v1, -0xc($s0)
    addu $v1, $v1, $v0
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    sll $v1, $v1, 5
    addu $v1, $v1, $v0
    lbu $v0, 0($v1)
    nop
    sb $v0, -9($s0)
    lbu $v0, 2($v1)
    nop
    sb $v0, -0xb($s0)
    lbu $v0, 3($v1)
    nop
    sb $v0, -0xa($s0)
    lbu $v0, 4($v1)
    nop
    sb $v0, -8($s0)
    lbu $v0, 5($v1)
    nop
    sb $v0, -7($s0)
    lbu $v0, 1($v1)
    nop
    sb $v0, -4($s0)
    lbu $v0, 6($v1)
    nop
    sb $v0, -6($s0)
    lbu $v0, 7($v1)
    addu $a0, $zero, $zero
    jal audio_sequence_allocate_voice
    sb $v0, -5($s0)
    andi $v0, $v0, 0xff
    lui $v1, %hi(audio_sequence_voice_count)
    lbu $v1, %lo(audio_sequence_voice_count)($v1)
    addu $a0, $v0, $zero
    slt $v1, $a0, $v1
    beqz $v1, .Lnotes_80171528
    sh $v0, 2($s0)
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    addiu $v1, $zero, 1
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $v0
    sb $v1, %lo(audio_sequence_voice_globals + 187)($at)
    lh $v1, 2($s0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 162)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 162)($at)
    lh $v1, 2($s0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 174)
    addu $at, $at, $v0
    sh $s5, %lo(audio_sequence_voice_globals + 174)($at)
    lh $v1, 2($s0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    lbu $v1, -0x17($s0)
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 182)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 182)($at)
    lh $v1, 2($s0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    lbu $v1, -0x11($s0)
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 176)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 176)($at)
    lh $v1, 2($s0)
    lhu $t0, 0x110($sp)
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 178)
    addu $at, $at, $v0
    sh $t0, %lo(audio_sequence_voice_globals + 178)($at)
    sll $v0, $s5, 0x10
    sra $v0, $v0, 0x10
    addiu $v1, $zero, 0x21
    beq $v0, $v1, .Lnotes_801713fc
    nop
    lh $v1, 2($s0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 168)
    addu $at, $at, $v0
    sh $s6, %lo(audio_sequence_voice_globals + 168)($at)
.Lnotes_801713fc:
    lh $v1, 2($s0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 170)
    addu $at, $at, $v0
    sb $fp, %lo(audio_sequence_voice_globals + 170)($at)
    lh $v1, 2($s0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    lbu $v1, -0xc($s0)
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 180)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 180)($at)
    lh $v1, 2($s0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 172)
    addu $at, $at, $v0
    sh $s7, %lo(audio_sequence_voice_globals + 172)($at)
    lh $v1, 2($s0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    lbu $v1, -9($s0)
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 184)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 184)($at)
    lh $v1, 2($s0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    lhu $v1, 0($s0)
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_records)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_records)($at)
    jal audio_sequence_prepare_voice
    nop
    lh $v1, 0($s0)
    addiu $v0, $zero, 0xff
    bne $v1, $v0, .Lnotes_801714fc
    nop
    lbu $a0, 0($s2)
    jal audio_sequence_key_noise
    nop
    j .Lnotes_80171510
    nop
.Lnotes_801714fc:
    jal audio_sequence_selected_pitch
    nop
    andi $a0, $s4, 0xff
    jal audio_sequence_key_voice
    andi $a1, $v0, 0xffff
.Lnotes_80171510:
    lui $v1, %hi(audio_sequence_active_voice)
    lh $v1, %lo(audio_sequence_active_voice)($v1)
    addiu $v0, $zero, 1
    sllv $v0, $v0, $v1
    j .Lnotes_8017152c
    or $s3, $s3, $v0
.Lnotes_80171528:
    addiu $s3, $zero, -1
.Lnotes_8017152c:
    addiu $s1, $s1, 1
    andi $v0, $s1, 0xff
    andi $v1, $s4, 0xff
    sltu $v0, $v0, $v1
    bnez $v0, .Lnotes_80171210
    andi $v1, $s1, 0xff
.Lnotes_80171544:
    addu $v0, $s3, $zero
.Lnotes_80171548:
    lw $ra, 0x144($sp)
    lw $fp, 0x140($sp)
    lw $s7, 0x13c($sp)
    lw $s6, 0x138($sp)
    lw $s5, 0x134($sp)
    lw $s4, 0x130($sp)
    lw $s3, 0x12c($sp)
    lw $s2, 0x128($sp)
    lw $s1, 0x124($sp)
    lw $s0, 0x120($sp)
    addiu $sp, $sp, 0x148
    jr $ra
    nop
.end audio_sequence_note_on

# exe/slus_004_22@0x8017157C..0x801716C8
.globl audio_sequence_note_off
.ent audio_sequence_note_off
audio_sequence_note_off:
    lui $v0, %hi(audio_sequence_voice_count)
    lbu $v0, %lo(audio_sequence_voice_count)($v0)
    addiu $sp, $sp, -0x38
    sw $s0, 0x18($sp)
    addu $s0, $zero, $zero
    sw $s1, 0x1c($sp)
    addu $s1, $zero, $zero
    sw $s5, 0x2c($sp)
    addu $s5, $a3, $zero
    sw $ra, 0x30($sp)
    sw $s4, 0x28($sp)
    sw $s3, 0x24($sp)
    beqz $v0, .Lnotes_8017169c
    sw $s2, 0x20($sp)
    sll $v0, $a2, 0x10
    sra $s4, $v0, 0x10
    sll $v0, $a0, 0x10
    sra $s3, $v0, 0x10
    sll $v0, $a1, 0x10
    sra $s2, $v0, 0x10
.Lnotes_801715cc:
    andi $a0, $s0, 0xff
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    addu $v0, $v0, $a0
    sll $a1, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 172)
    addu $at, $at, $a1
    lh $v1, %lo(audio_sequence_voice_globals + 172)($at)
    andi $v0, $s5, 0xffff
    bne $v1, $v0, .Lnotes_80171680
    nop
    lui $at, %hi(audio_sequence_voice_globals + 178)
    addu $at, $at, $a1
    lh $v0, %lo(audio_sequence_voice_globals + 178)($at)
    nop
    bne $v0, $s4, .Lnotes_80171680
    nop
    lui $at, %hi(audio_sequence_voice_globals + 174)
    addu $at, $at, $a1
    lh $v0, %lo(audio_sequence_voice_globals + 174)($at)
    nop
    bne $v0, $s3, .Lnotes_80171680
    nop
    lui $at, %hi(audio_sequence_voice_globals + 182)
    addu $at, $at, $a1
    lh $v0, %lo(audio_sequence_voice_globals + 182)($at)
    nop
    bne $v0, $s2, .Lnotes_80171680
    addiu $v0, $zero, 0xff
    lui $at, %hi(audio_sequence_voice_records)
    addu $at, $at, $a1
    lh $v1, %lo(audio_sequence_voice_records)($at)
    nop
    bne $v1, $v0, .Lnotes_8017166c
    andi $v0, $s0, 0xff
    jal audio_sequence_stop_noise
    addiu $s1, $s1, 1
    j .Lnotes_80171684
    addiu $s0, $s0, 1
.Lnotes_8017166c:
    lui $at, %hi(audio_sequence_active_voice)
    sh $v0, %lo(audio_sequence_active_voice)($at)
    jal audio_sequence_release_voice
    addu $a0, $zero, $zero
    addiu $s1, $s1, 1
.Lnotes_80171680:
    addiu $s0, $s0, 1
.Lnotes_80171684:
    lui $v1, %hi(audio_sequence_voice_count)
    lbu $v1, %lo(audio_sequence_voice_count)($v1)
    andi $v0, $s0, 0xff
    sltu $v0, $v0, $v1
    bnez $v0, .Lnotes_801715cc
    nop
.Lnotes_8017169c:
    addu $v0, $s1, $zero
    lw $ra, 0x30($sp)
    lw $s5, 0x2c($sp)
    lw $s4, 0x28($sp)
    lw $s3, 0x24($sp)
    lw $s2, 0x20($sp)
    lw $s1, 0x1c($sp)
    lw $s0, 0x18($sp)
    addiu $sp, $sp, 0x38
    jr $ra
    nop
.end audio_sequence_note_off

# exe/slus_004_22@0x801717F0..0x80171A5C
.globl audio_sequence_allocate_voice
.ent audio_sequence_allocate_voice
audio_sequence_allocate_voice:
    addiu $sp, $sp, -0x28
    sw $s0, 0x20($sp)
    addiu $s0, $zero, 0x63
    ori $t3, $zero, 0xffff
    addu $t2, $zero, $zero
    addu $t0, $zero, $zero
    addiu $t1, $zero, 0x63
    lui $v0, %hi(audio_sequence_voice_count)
    lbu $v0, %lo(audio_sequence_voice_count)($v0)
    lui $t4, %hi(audio_sequence_allocation_mask + 23)
    lbu $t4, %lo(audio_sequence_allocation_mask + 23)($t4)
    addu $a3, $zero, $zero
    beqz $v0, .Lnotes_80171954
    sw $ra, 0x24($sp)
    addu $t5, $v0, $zero
    andi $v0, $a3, 0xff
.Lnotes_80171830:
    sll $v1, $v0, 1
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $v1
    lbu $v0, %lo(audio_sequence_voice_globals + 187)($at)
    nop
    bnez $v0, .Lnotes_8017187c
    andi $v0, $a3, 0xff
    lui $at, %hi(audio_sequence_voice_globals + 166)
    addu $at, $at, $v1
    lhu $v0, %lo(audio_sequence_voice_globals + 166)($at)
    nop
    bnez $v0, .Lnotes_8017187c
    andi $v0, $a3, 0xff
    j .Lnotes_80171954
    addu $s0, $a3, $zero
.Lnotes_8017187c:
    sll $v1, $v0, 1
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    lui $at, %hi(audio_sequence_voice_globals + 184)
    addu $at, $at, $v1
    lh $a2, %lo(audio_sequence_voice_globals + 184)($at)
    andi $a0, $t4, 0xffff
    slt $v0, $a2, $a0
    beqz $v0, .Lnotes_801718d4
    addu $a1, $a2, $zero
    addu $t4, $a1, $zero
    addu $t1, $a3, $zero
    lui $at, %hi(audio_sequence_voice_globals + 166)
    addu $at, $at, $v1
    lhu $t3, %lo(audio_sequence_voice_globals + 166)($at)
    lui $at, %hi(audio_sequence_voice_globals + 162)
    addu $at, $at, $v1
    lhu $t0, %lo(audio_sequence_voice_globals + 162)($at)
    j .Lnotes_80171940
    addiu $t2, $zero, 1
.Lnotes_801718d4:
    bne $a2, $a0, .Lnotes_80171940
    andi $a0, $t3, 0xffff
    lui $at, %hi(audio_sequence_voice_globals + 166)
    addu $at, $at, $v1
    lhu $a2, %lo(audio_sequence_voice_globals + 166)($at)
    nop
    andi $a1, $a2, 0xffff
    sltu $v0, $a1, $a0
    beqz $v0, .Lnotes_80171910
    addiu $t2, $t2, 1
    lui $at, %hi(audio_sequence_voice_globals + 162)
    addu $at, $at, $v1
    lhu $t0, %lo(audio_sequence_voice_globals + 162)($at)
    j .Lnotes_8017193c
    addu $t3, $a2, $zero
.Lnotes_80171910:
    bne $a1, $a0, .Lnotes_80171940
    nop
    lui $at, %hi(audio_sequence_voice_globals + 162)
    addu $at, $at, $v1
    lh $v0, %lo(audio_sequence_voice_globals + 162)($at)
    nop
    addu $v1, $v0, $zero
    slt $v0, $t0, $v0
    beqz $v0, .Lnotes_80171940
    nop
    addu $t0, $v1, $zero
.Lnotes_8017193c:
    addu $t1, $a3, $zero
.Lnotes_80171940:
    addiu $a3, $a3, 1
    andi $v0, $a3, 0xff
    sltu $v0, $v0, $t5
    bnez $v0, .Lnotes_80171830
    andi $v0, $a3, 0xff
.Lnotes_80171954:
    andi $v1, $s0, 0xff
    addiu $v0, $zero, 0x63
    bne $v1, $v0, .Lnotes_80171974
    andi $v0, $t2, 0xff
    bnez $v0, .Lnotes_80171974
    addu $s0, $t1, $zero
    lui $s0, %hi(audio_sequence_voice_count)
    lbu $s0, %lo(audio_sequence_voice_count)($s0)
.Lnotes_80171974:
    lui $v1, %hi(audio_sequence_voice_count)
    lbu $v1, %lo(audio_sequence_voice_count)($v1)
    andi $v0, $s0, 0xff
    sltu $v0, $v0, $v1
    beqz $v0, .Lnotes_80171a44
    nop
    beqz $v1, .Lnotes_801719e4
    addu $a3, $zero, $zero
    lui $a1, %hi(audio_sequence_voice_records)
    addiu $a1, $a1, %lo(audio_sequence_voice_records)
    addu $a0, $v1, $zero
    andi $v1, $a3, 0xff
.Lnotes_801719a4:
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addiu $a3, $a3, 1
    lui $at, %hi(audio_sequence_voice_globals + 162)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_sequence_voice_globals + 162)($at)
    addu $v0, $v0, $a1
    addiu $v1, $v1, 1
    sh $v1, 2($v0)
    andi $v0, $a3, 0xff
    sltu $v0, $v0, $a0
    bnez $v0, .Lnotes_801719a4
    andi $v1, $a3, 0xff
.Lnotes_801719e4:
    andi $v1, $s0, 0xff
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 162)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 162)($at)
    lui $v1, %hi(audio_sequence_allocation_mask + 23)
    lbu $v1, %lo(audio_sequence_allocation_mask + 23)($v1)
    lui $at, %hi(audio_sequence_voice_globals + 184)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 184)($at)
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $v0
    lbu $v1, %lo(audio_sequence_voice_globals + 187)($at)
    addiu $v0, $zero, 2
    bne $v1, $v0, .Lnotes_80171a48
    andi $v0, $s0, 0xff
    lui $a1, 0xff
    ori $a1, $a1, 0xffff
    jal audio_spu_set_reverb_voice_mask
    addu $a0, $zero, $zero
.Lnotes_80171a44:
    andi $v0, $s0, 0xff
.Lnotes_80171a48:
    lw $ra, 0x24($sp)
    lw $s0, 0x20($sp)
    addiu $sp, $sp, 0x28
    jr $ra
    nop
.end audio_sequence_allocate_voice

# exe/slus_004_22@0x80171A5C..0x80171B20
.globl audio_sequence_selected_pitch
.ent audio_sequence_selected_pitch
audio_sequence_selected_pitch:
    lui $a1, %hi(audio_sequence_allocation_mask + 25)
    lbu $a1, %lo(audio_sequence_allocation_mask + 25)($a1)
    lui $v0, %hi(audio_sequence_allocation_mask + 10)
    lbu $v0, %lo(audio_sequence_allocation_mask + 10)($v0)
    lui $v1, %hi(audio_sequence_allocation_mask + 24)
    lbu $v1, %lo(audio_sequence_allocation_mask + 24)($v1)
    addiu $v0, $v0, 0x3c
    bgez $a1, .Lnotes_80171a84
    subu $v0, $v0, $v1
    addiu $a1, $a1, 7
.Lnotes_80171a84:
    lui $v1, 0x2aaa
    ori $v1, $v1, 0xaaab
    sll $v0, $v0, 0x10
    sra $a0, $v0, 0x10
    mult $a0, $v1
    srl $a1, $a1, 3
    sra $v0, $v0, 0x1f
    mfhi $a3
    sra $v1, $a3, 1
    subu $a2, $v1, $v0
    sll $v0, $a2, 1
    addu $v0, $v0, $a2
    sll $v0, $v0, 2
    subu $a0, $a0, $v0
    sltiu $v0, $a1, 0x10
    bnez $v0, .Lnotes_80171acc
    sll $v0, $a0, 0x10
    addiu $a1, $zero, 0xf
.Lnotes_80171acc:
    sra $v0, $v0, 0xc
    andi $v1, $a1, 0xffff
    addu $v0, $v0, $v1
    sll $v0, $v0, 1
    lui $at, %hi(audio_sequence_pitch_table)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_sequence_pitch_table)($at)
    addiu $v0, $a2, -5
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    blez $v0, .Lnotes_80171b04
    nop
    j .Lnotes_80171b18
    sllv $v1, $v1, $v0
.Lnotes_80171b04:
    bgez $v0, .Lnotes_80171b18
    nop
    andi $v1, $v1, 0xffff
    negu $v0, $v0
    srav $v1, $v1, $v0
.Lnotes_80171b18:
    jr $ra
    andi $v0, $v1, 0xffff
.end audio_sequence_selected_pitch

# exe/slus_004_22@0x80171C1C..0x801720E8
.globl audio_sequence_key_noise
.ent audio_sequence_key_noise
audio_sequence_key_noise:
    lui $v1, %hi(audio_sequence_voice_owner)
    lhu $v1, %lo(audio_sequence_voice_owner)($v1)
    addu $t2, $a0, $zero
    andi $a0, $v1, 0xff
    sll $a0, $a0, 2
    srl $v1, $v1, 8
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a0
    lw $a0, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $a0, $a0, $v0
    lhu $v0, 0x74($a0)
    lui $v1, %hi(audio_sequence_allocation_mask + 18)
    lbu $v1, %lo(audio_sequence_allocation_mask + 18)($v1)
    sll $a1, $v0, 7
    addu $a1, $a1, $v0
    mult $a1, $v1
    lhu $a0, 0x76($a0)
    mflo $a1
    sll $v0, $a0, 7
    addu $a3, $v0, $a0
    mult $a3, $v1
    mflo $v1
    lui $a2, 0x204
    ori $a2, $a2, 0x811
    multu $a1, $a2
    mfhi $v0
    nop
    nop
    multu $v1, $a2
    lui $a0, %hi(audio_sequence_allocation_mask + 21)
    lbu $a0, %lo(audio_sequence_allocation_mask + 21)($a0)
    subu $a1, $a1, $v0
    srl $a1, $a1, 1
    mfhi $a3
    addu $v0, $v0, $a1
    srl $a1, $v0, 6
    mult $a1, $a0
    subu $v1, $v1, $a3
    srl $v1, $v1, 1
    mflo $a1
    addu $a3, $a3, $v1
    srl $a3, $a3, 6
    mult $a3, $a0
    mflo $a0
    nop
    nop
    multu $a1, $a2
    mfhi $v0
    nop
    nop
    multu $a0, $a2
    addiu $sp, $sp, -8
    lui $v1, %hi(audio_sequence_registers_pointer)
    lw $v1, %lo(audio_sequence_registers_pointer)($v1)
    lui $t0, %hi(audio_sequence_allocation_mask + 22)
    lbu $t0, %lo(audio_sequence_allocation_mask + 22)($t0)
    lhu $t1, 0x1aa($v1)
    subu $a1, $a1, $v0
    srl $a1, $a1, 1
    addu $v0, $v0, $a1
    srl $a1, $v0, 6
    sltiu $v0, $t0, 0x40
    mfhi $a2
    subu $a0, $a0, $a2
    srl $a0, $a0, 1
    addu $a2, $a2, $a0
    beqz $v0, .Lnotes_80171d74
    srl $a3, $a2, 6
    mult $a3, $t0
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    j .Lnotes_80171da8
    srl $a2, $v1, 5
.Lnotes_80171d74:
    addiu $v0, $zero, 0x7f
    subu $v0, $v0, $t0
    mult $a1, $v0
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    addu $a2, $a3, $zero
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $a1, $v1, 5
.Lnotes_80171da8:
    lui $t0, %hi(audio_sequence_allocation_mask + 19)
    lbu $t0, %lo(audio_sequence_allocation_mask + 19)($t0)
    nop
    sltiu $v0, $t0, 0x40
    beqz $v0, .Lnotes_80171de8
    mult $a2, $t0
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    j .Lnotes_80171e18
    srl $a2, $v1, 5
.Lnotes_80171de8:
    addiu $v0, $zero, 0x7f
    subu $v0, $v0, $t0
    mult $a1, $v0
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $a1, $v1, 5
.Lnotes_80171e18:
    lui $t0, %hi(audio_sequence_allocation_mask + 13)
    lbu $t0, %lo(audio_sequence_allocation_mask + 13)($t0)
    nop
    sltiu $v0, $t0, 0x40
    beqz $v0, .Lnotes_80171e58
    mult $t0, $a2
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    j .Lnotes_80171e88
    srl $a2, $v1, 5
.Lnotes_80171e58:
    addiu $v0, $zero, 0x7f
    subu $v0, $v0, $t0
    mult $a1, $v0
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $a1, $v1, 5
.Lnotes_80171e88:
    lui $v1, %hi(audio_sequence_voice_control_e246)
    lh $v1, %lo(audio_sequence_voice_control_e246)($v1)
    addiu $v0, $zero, 1
    bne $v1, $v0, .Lnotes_80171eb0
    sltu $v0, $a1, $a2
    beqz $v0, .Lnotes_80171eac
    nop
    j .Lnotes_80171eb0
    addu $a1, $a2, $zero
.Lnotes_80171eac:
    addu $a2, $a1, $zero
.Lnotes_80171eb0:
    lui $v0, %hi(audio_sequence_allocation_mask + 10)
    lbu $v0, %lo(audio_sequence_allocation_mask + 10)($v0)
    lui $v1, %hi(audio_sequence_allocation_mask + 24)
    lbu $v1, %lo(audio_sequence_allocation_mask + 24)($v1)
    andi $a0, $t1, 0xc0ff
    subu $v0, $v0, $v1
    andi $v0, $v0, 0x3f
    sll $v0, $v0, 8
    or $t1, $a0, $v0
    lui $v1, %hi(audio_sequence_registers_pointer)
    lw $v1, %lo(audio_sequence_registers_pointer)($v1)
    andi $a0, $t2, 0xff
    sh $t1, 0x1aa($v1)
    sll $v1, $a0, 4
    lui $at, %hi(audio_sequence_voice_flags + 18)
    addu $at, $at, $v1
    sh $a2, %lo(audio_sequence_voice_flags + 18)($at)
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $a0
    lbu $v0, %lo(audio_sequence_voice_dirty)($at)
    lui $at, %hi(audio_sequence_voice_register_cache)
    addu $at, $at, $v1
    sh $a1, %lo(audio_sequence_voice_register_cache)($at)
    ori $v0, $v0, 3
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $a0
    sb $v0, %lo(audio_sequence_voice_dirty)($at)
    sltiu $v0, $a0, 0x10
    beqz $v0, .Lnotes_80171f34
    addiu $v0, $zero, 1
    sllv $a3, $v0, $a0
    j .Lnotes_80171f40
    addu $a2, $zero, $zero
.Lnotes_80171f34:
    addu $a3, $zero, $zero
    addiu $v1, $a0, -0x10
    sllv $a2, $v0, $v1
.Lnotes_80171f40:
    andi $v1, $t2, 0xff
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $a0, %hi(audio_sequence_voice_count)
    lbu $a0, %lo(audio_sequence_voice_count)($a0)
    addiu $v1, $zero, 0xa
    lui $at, %hi(audio_sequence_voice_globals + 164)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 164)($at)
    beqz $a0, .Lnotes_80171fd4
    addu $a1, $zero, $zero
    sll $v1, $a1, 0x10
.Lnotes_80171f7c:
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addiu $v1, $a1, 1
    addu $a1, $v1, $zero
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_sequence_voice_globals + 187)($at)
    sll $v1, $v1, 0x10
    andi $a0, $a0, 1
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $v0
    sb $a0, %lo(audio_sequence_voice_globals + 187)($at)
    lui $v0, %hi(audio_sequence_voice_count)
    lbu $v0, %lo(audio_sequence_voice_count)($v0)
    sra $v1, $v1, 0x10
    slt $v1, $v1, $v0
    bnez $v1, .Lnotes_80171f7c
    sll $v1, $a1, 0x10
.Lnotes_80171fd4:
    andi $v1, $t2, 0xff
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addiu $v1, $zero, 2
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $v0
    sb $v1, %lo(audio_sequence_voice_globals + 187)($at)
    lui $v1, %hi(audio_sequence_voice_globals)
    lhu $v1, %lo(audio_sequence_voice_globals)($v1)
    lui $a0, %hi(audio_sequence_voice_globals + 2)
    lhu $a0, %lo(audio_sequence_voice_globals + 2)($a0)
    lui $v0, %hi(audio_sequence_release_mask)
    lhu $v0, %lo(audio_sequence_release_mask)($v0)
    or $v1, $v1, $a3
    or $a0, $a0, $a2
    lui $at, %hi(audio_sequence_voice_globals)
    sh $v1, %lo(audio_sequence_voice_globals)($at)
    nor $v1, $zero, $v1
    and $v0, $v0, $v1
    lui $at, %hi(audio_sequence_voice_globals + 2)
    sh $a0, %lo(audio_sequence_voice_globals + 2)($at)
    nor $a0, $zero, $a0
    lui $at, %hi(audio_sequence_release_mask)
    sh $v0, %lo(audio_sequence_release_mask)($at)
    lui $v0, %hi(audio_sequence_envelope_history + 66)
    lhu $v0, %lo(audio_sequence_envelope_history + 66)($v0)
    lui $v1, %hi(audio_sequence_allocation_mask + 28)
    lbu $v1, %lo(audio_sequence_allocation_mask + 28)($v1)
    and $v0, $v0, $a0
    andi $v1, $v1, 4
    lui $at, %hi(audio_sequence_envelope_history + 66)
    sh $v0, %lo(audio_sequence_envelope_history + 66)($at)
    beqz $v1, .Lnotes_80172098
    nor $v1, $zero, $a3
    lui $v0, %hi(audio_sequence_voice_globals + 4)
    lhu $v0, %lo(audio_sequence_voice_globals + 4)($v0)
    lui $v1, %hi(audio_sequence_voice_globals + 6)
    lhu $v1, %lo(audio_sequence_voice_globals + 6)($v1)
    or $v0, $v0, $a3
    or $v1, $v1, $a2
    lui $at, %hi(audio_sequence_voice_globals + 4)
    sh $v0, %lo(audio_sequence_voice_globals + 4)($at)
    lui $at, %hi(audio_sequence_voice_globals + 6)
    sh $v1, %lo(audio_sequence_voice_globals + 6)($at)
    j .Lnotes_801720c8
    nop
.Lnotes_80172098:
    lui $v0, %hi(audio_sequence_voice_globals + 4)
    lhu $v0, %lo(audio_sequence_voice_globals + 4)($v0)
    nop
    and $v0, $v0, $v1
    lui $at, %hi(audio_sequence_voice_globals + 4)
    sh $v0, %lo(audio_sequence_voice_globals + 4)($at)
    lui $v0, %hi(audio_sequence_voice_globals + 6)
    lhu $v0, %lo(audio_sequence_voice_globals + 6)($v0)
    nor $v1, $zero, $a2
    and $v0, $v0, $v1
    lui $at, %hi(audio_sequence_voice_globals + 6)
    sh $v0, %lo(audio_sequence_voice_globals + 6)($at)
.Lnotes_801720c8:
    lui $v0, %hi(audio_sequence_registers_pointer)
    lw $v0, %lo(audio_sequence_registers_pointer)($v0)
    nop
    sh $a3, 0x194($v0)
    sh $a2, 0x196($v0)
    addiu $sp, $sp, 8
    jr $ra
    nop
.end audio_sequence_key_noise

# exe/slus_004_22@0x801720E8..0x8017212C
.globl audio_sequence_stop_noise
.ent audio_sequence_stop_noise
audio_sequence_stop_noise:
    andi $a0, $a0, 0xff
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $v0
    sb $zero, %lo(audio_sequence_voice_globals + 187)($at)
    lui $v1, %hi(audio_sequence_registers_pointer)
    lw $v1, %lo(audio_sequence_registers_pointer)($v1)
    lui $at, %hi(audio_sequence_voice_globals + 164)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 164)($at)
    sh $zero, 0x194($v1)
    jr $ra
    sh $zero, 0x196($v1)
.end audio_sequence_stop_noise

# exe/slus_004_22@0x801721FC..0x801726E0
.globl audio_sequence_key_voice
.ent audio_sequence_key_voice
audio_sequence_key_voice:
    lui $v0, %hi(audio_sequence_selected_header)
    lw $v0, %lo(audio_sequence_selected_header)($v0)
    nop
    lbu $a0, 0x18($v0)
    lui $v1, %hi(audio_sequence_selected_tone)
    lbu $v1, %lo(audio_sequence_selected_tone)($v1)
    sll $v0, $a0, 0xe
    subu $v0, $v0, $a0
    mult $v1, $v0
    mflo $v1
    lui $v0, 0x8206
    ori $v0, $v0, 0x1029
    mult $v1, $v0
    lui $a0, %hi(audio_sequence_allocation_mask + 18)
    lbu $a0, %lo(audio_sequence_allocation_mask + 18)($a0)
    mfhi $v0
    addu $v0, $v0, $v1
    sra $v0, $v0, 0xd
    sra $v1, $v1, 0x1f
    subu $a3, $v0, $v1
    mult $a3, $a0
    mflo $v1
    lui $v0, %hi(audio_sequence_allocation_mask + 21)
    lbu $v0, %lo(audio_sequence_allocation_mask + 21)($v0)
    nop
    mult $v1, $v0
    mflo $v0
    lui $a0, 0x40c
    ori $a0, $a0, 0x2051
    multu $v0, $a0
    addiu $sp, $sp, -0x10
    lui $v1, %hi(audio_sequence_active_voice)
    lh $v1, %lo(audio_sequence_active_voice)($v1)
    addu $t0, $a1, $zero
    sll $t1, $v1, 3
    lui $v1, %hi(audio_sequence_voice_owner)
    lhu $v1, %lo(audio_sequence_voice_owner)($v1)
    mfhi $a0
    subu $v0, $v0, $a0
    srl $v0, $v0, 1
    addu $a0, $a0, $v0
    srl $a2, $a0, 0xd
    andi $a0, $v1, 0xff
    sll $a0, $a0, 2
    sll $v1, $v1, 0x10
    sra $a1, $v1, 0x10
    srl $v1, $v1, 0x18
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a0
    lw $v1, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $v1, $v1, $v0
    addiu $v0, $zero, 0x21
    beq $a1, $v0, .Lnotes_80172350
    addu $a3, $a2, $zero
    lhu $v0, 0x74($v1)
    nop
    mult $a2, $v0
    mflo $v0
    lhu $v1, 0x76($v1)
    nop
    mult $a2, $v1
    mflo $v1
    lui $a0, 0x204
    ori $a0, $a0, 0x811
    multu $v0, $a0
    mfhi $a1
    nop
    nop
    multu $v1, $a0
    subu $v0, $v0, $a1
    srl $v0, $v0, 1
    addu $a1, $a1, $v0
    srl $a3, $a1, 6
    mfhi $a0
    subu $v1, $v1, $a0
    srl $v1, $v1, 1
    addu $a0, $a0, $v1
    srl $a2, $a0, 6
.Lnotes_80172350:
    lui $v1, %hi(audio_sequence_allocation_mask + 22)
    lbu $v1, %lo(audio_sequence_allocation_mask + 22)($v1)
    nop
    sltiu $v0, $v1, 0x40
    beqz $v0, .Lnotes_80172394
    mult $a2, $v1
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    addu $a1, $a3, $zero
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    j .Lnotes_801723c4
    srl $a2, $v1, 5
.Lnotes_80172394:
    addiu $v0, $zero, 0x7f
    subu $v0, $v0, $v1
    mult $a3, $v0
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $a1, $v1, 5
.Lnotes_801723c4:
    lui $v1, %hi(audio_sequence_allocation_mask + 19)
    lbu $v1, %lo(audio_sequence_allocation_mask + 19)($v1)
    nop
    sltiu $v0, $v1, 0x40
    beqz $v0, .Lnotes_80172404
    mult $a2, $v1
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    j .Lnotes_80172434
    srl $a2, $v1, 5
.Lnotes_80172404:
    addiu $v0, $zero, 0x7f
    subu $v0, $v0, $v1
    mult $a1, $v0
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $a1, $v1, 5
.Lnotes_80172434:
    lui $v1, %hi(audio_sequence_allocation_mask + 13)
    lbu $v1, %lo(audio_sequence_allocation_mask + 13)($v1)
    nop
    sltiu $v0, $v1, 0x40
    beqz $v0, .Lnotes_80172474
    mult $a2, $v1
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    j .Lnotes_801724a4
    srl $a2, $v1, 5
.Lnotes_80172474:
    addiu $v0, $zero, 0x7f
    subu $v0, $v0, $v1
    mult $a1, $v0
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $a1, $v1, 5
.Lnotes_801724a4:
    lui $v1, %hi(audio_sequence_voice_control_e246)
    lh $v1, %lo(audio_sequence_voice_control_e246)($v1)
    addiu $v0, $zero, 1
    bne $v1, $v0, .Lnotes_801724cc
    sltu $v0, $a1, $a2
    beqz $v0, .Lnotes_801724c8
    nop
    j .Lnotes_801724cc
    addu $a1, $a2, $zero
.Lnotes_801724c8:
    addu $a2, $a1, $zero
.Lnotes_801724cc:
    mult $a1, $a1
    mflo $v1
    nop
    nop
    mult $a2, $a2
    mflo $a2
    lui $v0, 4
    ori $v0, $v0, 0x11
    multu $v1, $v0
    mfhi $a1
    nop
    nop
    multu $a2, $v0
    andi $a0, $t1, 0xffff
    sll $a0, $a0, 1
    lui $at, %hi(audio_sequence_voice_flags + 20)
    addu $at, $at, $a0
    sh $t0, %lo(audio_sequence_voice_flags + 20)($at)
    subu $v1, $v1, $a1
    srl $v1, $v1, 1
    addu $a1, $a1, $v1
    srl $a1, $a1, 0xd
    lui $v1, %hi(audio_sequence_voice_flags + 20)
    addiu $v1, $v1, %lo(audio_sequence_voice_flags + 20)
    mfhi $v0
    subu $a2, $a2, $v0
    srl $a2, $a2, 1
    addu $v0, $v0, $a2
    srl $a2, $v0, 0xd
    addiu $v0, $v1, -4
    addu $v0, $a0, $v0
    addiu $v1, $v1, -2
    addu $a0, $a0, $v1
    sh $a1, 0($v0)
    sh $a2, 0($a0)
    lui $a0, %hi(audio_sequence_active_voice)
    addiu $a0, $a0, %lo(audio_sequence_active_voice)
    lh $v1, 0($a0)
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $v1
    lbu $v0, %lo(audio_sequence_voice_dirty)($at)
    nop
    ori $v0, $v0, 7
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $v1
    sb $v0, %lo(audio_sequence_voice_dirty)($at)
    lh $v1, 0($a0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 164)
    addu $at, $at, $v0
    sh $t0, %lo(audio_sequence_voice_globals + 164)($at)
    lh $v1, 0($a0)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addiu $v1, $zero, 1
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $v0
    sb $v1, %lo(audio_sequence_voice_globals + 187)($at)
    lh $v1, 0($a0)
    nop
    slti $v0, $v1, 0x10
    beqz $v0, .Lnotes_801725f8
    addiu $v0, $zero, 1
    sllv $a2, $v0, $v1
    j .Lnotes_80172604
    addu $a1, $zero, $zero
.Lnotes_801725f8:
    addu $a2, $zero, $zero
    addiu $v1, $v1, -0x10
    sllv $a1, $v0, $v1
.Lnotes_80172604:
    lui $v0, %hi(audio_sequence_allocation_mask + 28)
    lbu $v0, %lo(audio_sequence_allocation_mask + 28)($v0)
    nop
    andi $v0, $v0, 4
    beqz $v0, .Lnotes_8017264c
    nor $v1, $zero, $a2
    lui $v0, %hi(audio_sequence_voice_globals + 4)
    lhu $v0, %lo(audio_sequence_voice_globals + 4)($v0)
    lui $v1, %hi(audio_sequence_voice_globals + 6)
    lhu $v1, %lo(audio_sequence_voice_globals + 6)($v1)
    or $v0, $v0, $a2
    or $v1, $v1, $a1
    lui $at, %hi(audio_sequence_voice_globals + 4)
    sh $v0, %lo(audio_sequence_voice_globals + 4)($at)
    lui $at, %hi(audio_sequence_voice_globals + 6)
    sh $v1, %lo(audio_sequence_voice_globals + 6)($at)
    j .Lnotes_8017267c
    nop
.Lnotes_8017264c:
    lui $v0, %hi(audio_sequence_voice_globals + 4)
    lhu $v0, %lo(audio_sequence_voice_globals + 4)($v0)
    nop
    and $v0, $v0, $v1
    lui $at, %hi(audio_sequence_voice_globals + 4)
    sh $v0, %lo(audio_sequence_voice_globals + 4)($at)
    lui $v0, %hi(audio_sequence_voice_globals + 6)
    lhu $v0, %lo(audio_sequence_voice_globals + 6)($v0)
    nor $v1, $zero, $a1
    and $v0, $v0, $v1
    lui $at, %hi(audio_sequence_voice_globals + 6)
    sh $v0, %lo(audio_sequence_voice_globals + 6)($at)
.Lnotes_8017267c:
    lui $v1, %hi(audio_sequence_voice_globals)
    lhu $v1, %lo(audio_sequence_voice_globals)($v1)
    lui $a0, %hi(audio_sequence_voice_globals + 2)
    lhu $a0, %lo(audio_sequence_voice_globals + 2)($a0)
    lui $v0, %hi(audio_sequence_release_mask)
    lhu $v0, %lo(audio_sequence_release_mask)($v0)
    or $v1, $v1, $a2
    lui $at, %hi(audio_sequence_voice_globals)
    sh $v1, %lo(audio_sequence_voice_globals)($at)
    nor $v1, $zero, $v1
    and $v0, $v0, $v1
    lui $at, %hi(audio_sequence_release_mask)
    sh $v0, %lo(audio_sequence_release_mask)($at)
    lui $v0, %hi(audio_sequence_envelope_history + 66)
    lhu $v0, %lo(audio_sequence_envelope_history + 66)($v0)
    or $a0, $a0, $a1
    lui $at, %hi(audio_sequence_voice_globals + 2)
    sh $a0, %lo(audio_sequence_voice_globals + 2)($at)
    nor $a0, $zero, $a0
    and $v0, $v0, $a0
    lui $at, %hi(audio_sequence_envelope_history + 66)
    sh $v0, %lo(audio_sequence_envelope_history + 66)($at)
    addiu $sp, $sp, 0x10
    jr $ra
    nop
.end audio_sequence_key_voice
