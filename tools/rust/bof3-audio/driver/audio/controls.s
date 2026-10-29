# SPDX-License-Identifier: MIT
# Owned sequence gain, pan, bend and live bank/tone selection.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Preserve failed selection, shared aliases and original integer arithmetic.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x80169EE8..0x80169FC0
.globl audio_sequence_volume_command
.ent audio_sequence_volume_command
audio_sequence_volume_command:
    addiu $sp, $sp, -0x30
    sw $s4, 0x28($sp)
    sll $s4, $a0, 0x10
    sra $s4, $s4, 0x10
    sll $v1, $s4, 2
    sw $s2, 0x20($sp)
    sll $s2, $a1, 0x10
    sra $s2, $s2, 0x10
    sll $v0, $s2, 1
    addu $v0, $v0, $s2
    sll $v0, $v0, 2
    subu $v0, $v0, $s2
    sll $v0, $v0, 2
    subu $v0, $v0, $s2
    sll $v0, $v0, 2
    sll $a1, $a1, 8
    or $a0, $a0, $a1
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    sw $s0, 0x18($sp)
    addu $s0, $a2, $zero
    sw $ra, 0x2c($sp)
    sw $s3, 0x24($sp)
    sw $s1, 0x1c($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v1
    lw $s1, %lo(audio_sequence_handles)($at)
    andi $a3, $s0, 0xff
    addu $s1, $s1, $v0
    lbu $s3, 0x12($s1)
    lh $a1, 0x4c($s1)
    addu $v0, $s1, $s3
    lbu $v1, 0x2c($v0)
    lbu $v0, 0x17($v0)
    addu $a2, $v1, $zero
    jal audio_sequence_set_program_gain
    sw $v0, 0x10($sp)
    addu $a0, $s4, $zero
    addu $a1, $s2, $zero
    sll $s3, $s3, 1
    addu $s3, $s3, $s1
    andi $s0, $s0, 0xff
    jal audio_sequence_read_delta
    sh $s0, 0x4e($s3)
    sw $v0, 0x88($s1)
    lw $ra, 0x2c($sp)
    lw $s4, 0x28($sp)
    lw $s3, 0x24($sp)
    lw $s2, 0x20($sp)
    lw $s1, 0x1c($sp)
    lw $s0, 0x18($sp)
    addiu $sp, $sp, 0x30
    jr $ra
    nop
.end audio_sequence_volume_command

# exe/slus_004_22@0x80169FC0..0x8016A090
.globl audio_sequence_pan_command
.ent audio_sequence_pan_command
audio_sequence_pan_command:
    addiu $sp, $sp, -0x30
    sw $s2, 0x20($sp)
    sll $s2, $a0, 0x10
    sra $s2, $s2, 0x10
    sll $v1, $s2, 2
    sw $s1, 0x1c($sp)
    sll $s1, $a1, 0x10
    sra $s1, $s1, 0x10
    sll $v0, $s1, 1
    addu $v0, $v0, $s1
    sll $v0, $v0, 2
    subu $v0, $v0, $s1
    sll $v0, $v0, 2
    subu $v0, $v0, $s1
    sll $v0, $v0, 2
    sll $a1, $a1, 8
    or $a0, $a0, $a1
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    sw $ra, 0x2c($sp)
    sw $s4, 0x28($sp)
    sw $s3, 0x24($sp)
    sw $s0, 0x18($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v1
    lw $s0, %lo(audio_sequence_handles)($at)
    addu $s4, $a2, $zero
    addu $s0, $s0, $v0
    lbu $v0, 0x12($s0)
    lh $a1, 0x4c($s0)
    addu $s3, $s0, $v0
    sll $v0, $v0, 1
    addu $v0, $v0, $s0
    lbu $a2, 0x2c($s3)
    lhu $a3, 0x4e($v0)
    andi $v0, $s4, 0xff
    jal audio_sequence_set_program_gain
    sw $v0, 0x10($sp)
    addu $a0, $s2, $zero
    addu $a1, $s1, $zero
    jal audio_sequence_read_delta
    sb $s4, 0x17($s3)
    sw $v0, 0x88($s0)
    lw $ra, 0x2c($sp)
    lw $s4, 0x28($sp)
    lw $s3, 0x24($sp)
    lw $s2, 0x20($sp)
    lw $s1, 0x1c($sp)
    lw $s0, 0x18($sp)
    addiu $sp, $sp, 0x30
    jr $ra
    nop
.end audio_sequence_pan_command

# exe/slus_004_22@0x8016A4A4..0x8016A55C
.globl audio_sequence_bend_command
.ent audio_sequence_bend_command
audio_sequence_bend_command:
    addiu $sp, $sp, -0x20
    addu $v1, $a1, $zero
    sw $s2, 0x18($sp)
    sll $s2, $a0, 0x10
    sra $s2, $s2, 0x10
    sll $a1, $s2, 2
    sw $s1, 0x14($sp)
    sll $s1, $v1, 0x10
    sra $s1, $s1, 0x10
    sll $v0, $s1, 1
    addu $v0, $v0, $s1
    sll $v0, $v0, 2
    subu $v0, $v0, $s1
    sll $v0, $v0, 2
    subu $v0, $v0, $s1
    sll $v0, $v0, 2
    sll $v1, $v1, 8
    or $a0, $a0, $v1
    sw $ra, 0x1c($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a1
    lw $s0, %lo(audio_sequence_handles)($at)
    sll $a0, $a0, 0x10
    addu $s0, $s0, $v0
    lw $a3, 4($s0)
    lh $a1, 0x4c($s0)
    addiu $v0, $a3, 1
    sw $v0, 4($s0)
    lbu $v0, 0x12($s0)
    lbu $a3, 0($a3)
    addu $v0, $v0, $s0
    lbu $a2, 0x2c($v0)
    jal audio_sequence_bend_program
    sra $a0, $a0, 0x10
    addu $a0, $s2, $zero
    jal audio_sequence_read_delta
    addu $a1, $s1, $zero
    sw $v0, 0x88($s0)
    lw $ra, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_bend_command

# exe/slus_004_22@0x80171B20..0x80171C1C
.globl audio_sequence_note_to_pitch
.ent audio_sequence_note_to_pitch
audio_sequence_note_to_pitch:
    lui $v0, %hi(audio_sequence_selected_tone_block)
    lbu $v0, %lo(audio_sequence_selected_tone_block)($v0)
    lui $v1, %hi(audio_sequence_allocation_mask + 20)
    lbu $v1, %lo(audio_sequence_allocation_mask + 20)($v1)
    sll $v0, $v0, 4
    addu $v1, $v1, $v0
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    sll $v1, $v1, 5
    addu $v1, $v1, $v0
    lbu $v0, 5($v1)
    andi $a1, $a1, 0xffff
    addu $v0, $a1, $v0
    bgez $v0, .Lcontrols_80171b60
    addu $t0, $a0, $zero
    addiu $v0, $v0, 7
.Lcontrols_80171b60:
    sra $a1, $v0, 3
    addu $a3, $a1, $zero
    slti $v0, $a1, 0x10
    bnez $v0, .Lcontrols_80171b7c
    addu $a2, $zero, $zero
    addiu $a2, $zero, 1
    addiu $a3, $a1, -0x10
.Lcontrols_80171b7c:
    lui $a0, 0x2aaa
    ori $a0, $a0, 0xaaab
    lbu $v1, 4($v1)
    addiu $v0, $t0, 0x3c
    subu $v0, $v0, $v1
    addu $v0, $a2, $v0
    sll $v0, $v0, 0x10
    sra $v1, $v0, 0x10
    mult $v1, $a0
    sra $v0, $v0, 0x1f
    mfhi $t1
    sra $a0, $t1, 1
    subu $a0, $a0, $v0
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v1, $v1, $v0
    sll $v1, $v1, 0x10
    sra $v1, $v1, 0xc
    sll $v0, $a3, 0x10
    sra $v0, $v0, 0x10
    addu $v1, $v1, $v0
    sll $v1, $v1, 1
    addiu $a0, $a0, -5
    sll $a0, $a0, 0x10
    sra $v0, $a0, 0x10
    lui $at, %hi(audio_sequence_pitch_table)
    addu $at, $at, $v1
    lhu $v1, %lo(audio_sequence_pitch_table)($at)
    blez $v0, .Lcontrols_80171c00
    nop
    j .Lcontrols_80171c14
    sllv $v1, $v1, $v0
.Lcontrols_80171c00:
    bgez $v0, .Lcontrols_80171c14
    nop
    andi $v1, $v1, 0xffff
    negu $v0, $v0
    srav $v1, $v1, $v0
.Lcontrols_80171c14:
    jr $ra
    andi $v0, $v1, 0xffff
.end audio_sequence_note_to_pitch

# exe/slus_004_22@0x801726E0..0x801728E0
.globl audio_sequence_bend_voice
.ent audio_sequence_bend_voice
audio_sequence_bend_voice:
    addiu $sp, $sp, -0x18
    addu $t1, $a0, $zero
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    lw $v0, 0x28($sp)
    sll $a1, $a1, 0x10
    sw $ra, 0x14($sp)
    sw $s0, 0x10($sp)
    addiu $t0, $v0, -0x40
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    addu $v0, $v0, $a0
    sll $a0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 174)
    addu $at, $at, $a0
    lh $v0, %lo(audio_sequence_voice_globals + 174)($at)
    sra $a1, $a1, 0x10
    bne $v0, $a1, .Lcontrols_801728cc
    addu $v0, $zero, $zero
    lui $at, %hi(audio_sequence_voice_globals + 182)
    addu $at, $at, $a0
    lh $v1, %lo(audio_sequence_voice_globals + 182)($at)
    sll $v0, $a2, 0x10
    sra $v0, $v0, 0x10
    bne $v1, $v0, .Lcontrols_801728cc
    addu $v0, $zero, $zero
    lui $at, %hi(audio_sequence_voice_globals + 178)
    addu $at, $at, $a0
    lh $v1, %lo(audio_sequence_voice_globals + 178)($at)
    sll $v0, $a3, 0x10
    sra $v0, $v0, 0x10
    bne $v1, $v0, .Lcontrols_801728cc
    addu $v0, $zero, $zero
    lui $v0, %hi(audio_sequence_selected_tone_block)
    lbu $v0, %lo(audio_sequence_selected_tone_block)($v0)
    lui $at, %hi(audio_sequence_voice_globals + 180)
    addu $at, $at, $a0
    lhu $v1, %lo(audio_sequence_voice_globals + 180)($at)
    lui $at, %hi(audio_sequence_voice_globals + 172)
    addu $at, $at, $a0
    lhu $a2, %lo(audio_sequence_voice_globals + 172)($at)
    sll $v0, $v0, 4
    addu $v1, $v1, $v0
    sll $v0, $t0, 0x10
    sra $a1, $v0, 0x10
    blez $a1, .Lcontrols_801727f8
    andi $v0, $v1, 0xffff
    lui $v1, %hi(audio_sequence_selected_tones)
    lw $v1, %lo(audio_sequence_selected_tones)($v1)
    sll $v0, $v0, 5
    addu $v0, $v0, $v1
    lbu $a0, 0xd($v0)
    nop
    mult $a1, $a0
    mflo $a0
    lui $v0, 0x8208
    ori $v0, $v0, 0x2083
    mult $a0, $v0
    sra $v1, $a0, 0x1f
    mfhi $t2
    addu $v0, $t2, $a0
    sra $v0, $v0, 5
    subu $v0, $v0, $v1
    addu $a2, $a2, $v0
    sll $v1, $v0, 6
    subu $v1, $v1, $v0
    subu $a0, $a0, $v1
    j .Lcontrols_80172854
    sll $a1, $a0, 1
.Lcontrols_801727f8:
    bgez $a1, .Lcontrols_80172850
    nop
    andi $v0, $v1, 0xffff
    lui $v1, %hi(audio_sequence_selected_tones)
    lw $v1, %lo(audio_sequence_selected_tones)($v1)
    sll $v0, $v0, 5
    addu $v0, $v0, $v1
    lbu $v0, 0xc($v0)
    nop
    mult $a1, $v0
    mflo $a0
    bgez $a0, .Lcontrols_80172830
    addu $v0, $a0, $zero
    addiu $v0, $a0, 0x3f
.Lcontrols_80172830:
    sra $v0, $v0, 6
    addu $v1, $a2, $v0
    addiu $a2, $v1, -1
    sll $v0, $v0, 6
    subu $v0, $a0, $v0
    sll $v0, $v0, 1
    j .Lcontrols_80172854
    addiu $a1, $v0, 0x7f
.Lcontrols_80172850:
    addu $a1, $zero, $zero
.Lcontrols_80172854:
    sll $s0, $t1, 0x10
    sra $s0, $s0, 0x10
    sll $v0, $s0, 1
    addu $v0, $v0, $s0
    sll $v0, $v0, 2
    addu $v0, $v0, $s0
    sll $v0, $v0, 2
    andi $a0, $a2, 0xffff
    lui $at, %hi(audio_sequence_voice_globals + 180)
    addu $at, $at, $v0
    lhu $v0, %lo(audio_sequence_voice_globals + 180)($at)
    lui $at, %hi(audio_sequence_active_voice)
    sh $t1, %lo(audio_sequence_active_voice)($at)
    lui $at, %hi(audio_sequence_allocation_mask + 20)
    sb $v0, %lo(audio_sequence_allocation_mask + 20)($at)
    jal audio_sequence_note_to_pitch
    andi $a1, $a1, 0xffff
    sll $v1, $s0, 4
    lui $at, %hi(audio_sequence_voice_flags + 20)
    addu $at, $at, $v1
    sh $v0, %lo(audio_sequence_voice_flags + 20)($at)
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $s0
    lbu $v1, %lo(audio_sequence_voice_dirty)($at)
    nop
    ori $v1, $v1, 4
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $s0
    sb $v1, %lo(audio_sequence_voice_dirty)($at)
    addiu $v0, $zero, 1
.Lcontrols_801728cc:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_bend_voice

# exe/slus_004_22@0x801728E0..0x801729D0
.globl audio_sequence_bend_program
.ent audio_sequence_bend_program
audio_sequence_bend_program:
    addiu $sp, $sp, -0x40
    sw $s2, 0x28($sp)
    addu $s2, $a0, $zero
    addu $v0, $a1, $zero
    sll $a0, $v0, 0x10
    sra $a0, $a0, 0x10
    sll $a1, $a2, 0x10
    sra $a1, $a1, 0x10
    sw $s5, 0x34($sp)
    addu $s5, $v0, $zero
    sw $s4, 0x30($sp)
    addu $s4, $a2, $zero
    sw $s3, 0x2c($sp)
    addu $s3, $a3, $zero
    sw $ra, 0x38($sp)
    sw $s1, 0x24($sp)
    jal audio_sequence_select_bank_program
    sw $s0, 0x20($sp)
    addu $s0, $zero, $zero
    lui $v1, %hi(audio_sequence_voice_globals + 1812)
    lbu $v1, %lo(audio_sequence_voice_globals + 1812)($v1)
    addu $v0, $s2, $zero
    lui $at, %hi(audio_sequence_voice_owner)
    sh $v0, %lo(audio_sequence_voice_owner)($at)
    blez $v1, .Lcontrols_801729a4
    addu $s1, $zero, $zero
    sll $s2, $s2, 0x10
    sll $a0, $s0, 0x10
.Lcontrols_80172950:
    sra $a0, $a0, 0x10
    sra $a1, $s2, 0x10
    andi $v0, $s3, 0xffff
    sll $a2, $s5, 0x10
    sll $a3, $s4, 0x10
    sra $a2, $a2, 0x10
    sra $a3, $a3, 0x10
    jal audio_sequence_bend_voice
    sw $v0, 0x10($sp)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    addu $s1, $s1, $v0
    addiu $v0, $s0, 1
    addu $s0, $v0, $zero
    sll $v0, $v0, 0x10
    lui $v1, %hi(audio_sequence_voice_globals + 1812)
    lbu $v1, %lo(audio_sequence_voice_globals + 1812)($v1)
    sra $v0, $v0, 0x10
    slt $v0, $v0, $v1
    bnez $v0, .Lcontrols_80172950
    sll $a0, $s0, 0x10
.Lcontrols_801729a4:
    addu $v0, $s1, $zero
    lw $ra, 0x38($sp)
    lw $s5, 0x34($sp)
    lw $s4, 0x30($sp)
    lw $s3, 0x2c($sp)
    lw $s2, 0x28($sp)
    lw $s1, 0x24($sp)
    lw $s0, 0x20($sp)
    addiu $sp, $sp, 0x40
    jr $ra
    nop
.end audio_sequence_bend_program

# exe/slus_004_22@0x801729D0..0x80172F10
.globl audio_sequence_set_gain
.ent audio_sequence_set_gain
audio_sequence_set_gain:
    addiu $sp, $sp, -0x18
    addu $t0, $a1, $zero
    andi $v0, $a0, 0xff
    sll $v0, $v0, 2
    addu $t7, $a0, $zero
    andi $a0, $a0, 0xff00
    sra $a0, $a0, 8
    sw $s1, 0x14($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v0
    lw $v1, %lo(audio_sequence_handles)($at)
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_owner)
    sh $t7, %lo(audio_sequence_voice_owner)($at)
    addu $t1, $v1, $v0
    andi $v0, $t0, 0xffff
    bnez $v0, .Lcontrols_80172a38
    andi $v0, $a2, 0xffff
    addiu $t0, $zero, 1
.Lcontrols_80172a38:
    bnez $v0, .Lcontrols_80172a44
    nop
    addiu $a2, $zero, 1
.Lcontrols_80172a44:
    sh $t0, 0x74($t1)
    lhu $v0, 0x74($t1)
    nop
    sltiu $v0, $v0, 0x80
    bnez $v0, .Lcontrols_80172a64
    sh $a2, 0x76($t1)
    addiu $v0, $zero, 0x7f
    sh $v0, 0x74($t1)
.Lcontrols_80172a64:
    lhu $v0, 0x76($t1)
    nop
    sltiu $v0, $v0, 0x80
    bnez $v0, .Lcontrols_80172a84
    sll $v0, $a3, 0x10
    addiu $v0, $zero, 0x7f
    sh $v0, 0x76($t1)
    sll $v0, $a3, 0x10
.Lcontrols_80172a84:
    sra $v0, $v0, 0x10
    addiu $v1, $zero, 1
    bne $v0, $v1, .Lcontrols_80172ef4
    nop
    lui $v0, %hi(audio_sequence_voice_globals + 1812)
    lbu $v0, %lo(audio_sequence_voice_globals + 1812)($v0)
    nop
    beqz $v0, .Lcontrols_80172ef4
    addu $t2, $zero, $zero
    lui $t5, 0x204
    ori $t5, $t5, 0x811
    addiu $t6, $zero, 0x7f
    lui $t3, 0x8208
    ori $t3, $t3, 0x2083
    lui $t4, 0x8002
    ori $t4, $t4, 9
    lui $t8, %hi(audio_sequence_voice_register_cache)
    addiu $t8, $t8, %lo(audio_sequence_voice_register_cache)
    addiu $t9, $t8, 2
    sll $v1, $t2, 0x10
.Lcontrols_80172ad4:
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $t0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 174)
    addu $at, $at, $t0
    lhu $v1, %lo(audio_sequence_voice_globals + 174)($at)
    andi $v0, $t7, 0xffff
    bne $v1, $v0, .Lcontrols_80172ed4
    addiu $v0, $t2, 1
    lbu $v0, 0x12($t1)
    lui $at, %hi(audio_sequence_voice_globals + 168)
    addu $at, $at, $t0
    lh $v1, %lo(audio_sequence_voice_globals + 168)($at)
    sll $v0, $v0, 1
    addu $v0, $v0, $t1
    lh $v0, 0x4e($v0)
    nop
    mult $v1, $v0
    mflo $v1
    lui $v0, 0x8102
    ori $v0, $v0, 0x409
    mult $v1, $v0
    mfhi $s1
    addu $a0, $s1, $v1
    sra $a0, $a0, 6
    sra $v1, $v1, 0x1f
    subu $a0, $a0, $v1
    lui $v1, %hi(audio_sequence_selected_header)
    lw $v1, %lo(audio_sequence_selected_header)($v1)
    sll $v0, $a0, 0xe
    lbu $v1, 0x18($v1)
    subu $v0, $v0, $a0
    mult $v1, $v0
    mflo $v1
    lui $v0, 0x8206
    ori $v0, $v0, 0x1029
    mult $v1, $v0
    lui $at, %hi(audio_sequence_voice_globals + 178)
    addu $at, $at, $t0
    lh $v0, %lo(audio_sequence_voice_globals + 178)($at)
    lui $a0, %hi(audio_sequence_selected_programs)
    lw $a0, %lo(audio_sequence_selected_programs)($a0)
    sll $v0, $v0, 4
    addu $v0, $v0, $a0
    lbu $v0, 1($v0)
    mfhi $s1
    addu $a1, $s1, $v1
    sra $a1, $a1, 0xd
    sra $v1, $v1, 0x1f
    subu $a3, $a1, $v1
    mult $a3, $v0
    lui $at, %hi(audio_sequence_voice_globals + 176)
    addu $at, $at, $t0
    lh $a2, %lo(audio_sequence_voice_globals + 176)($at)
    lui $at, %hi(audio_sequence_voice_globals + 180)
    addu $at, $at, $t0
    lh $v0, %lo(audio_sequence_voice_globals + 180)($at)
    sll $a2, $a2, 4
    addu $a2, $a2, $v0
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    sll $a2, $a2, 5
    addu $a2, $a2, $v0
    mflo $v1
    lbu $v0, 2($a2)
    nop
    mult $v1, $v0
    mflo $v0
    lui $v1, 0x40c
    ori $v1, $v1, 0x2051
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    lhu $v0, 0x74($t1)
    srl $a0, $v1, 0xd
    mult $a0, $v0
    mflo $v0
    lhu $v1, 0x76($t1)
    nop
    mult $a0, $v1
    mflo $v1
    nop
    nop
    multu $v0, $t5
    mfhi $a0
    nop
    nop
    multu $v1, $t5
    subu $v0, $v0, $a0
    srl $v0, $v0, 1
    addu $a0, $a0, $v0
    srl $a3, $a0, 6
    mfhi $a1
    subu $v1, $v1, $a1
    srl $v1, $v1, 1
    addu $a1, $a1, $v1
    lbu $v1, 3($a2)
    nop
    sltiu $v0, $v1, 0x40
    beqz $v0, .Lcontrols_80172cac
    srl $a0, $a1, 6
    mult $a0, $v1
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    addu $t0, $a3, $zero
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    j .Lcontrols_80172cdc
    srl $a2, $v1, 5
.Lcontrols_80172cac:
    subu $v0, $t6, $v1
    mult $a3, $v0
    mflo $v0
    lui $v1, 0x410
    ori $v1, $v1, 0x4105
    multu $v0, $v1
    addu $a2, $a0, $zero
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $t0, $v1, 5
.Lcontrols_80172cdc:
    sll $v1, $t2, 0x10
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 178)
    addu $at, $at, $v0
    lh $v0, %lo(audio_sequence_voice_globals + 178)($at)
    lui $v1, %hi(audio_sequence_selected_programs)
    lw $v1, %lo(audio_sequence_selected_programs)($v1)
    sll $v0, $v0, 4
    addu $v0, $v0, $v1
    lbu $v1, 4($v0)
    nop
    sltiu $v0, $v1, 0x40
    beqz $v0, .Lcontrols_80172d4c
    andi $v0, $a2, 0xffff
    mult $v0, $v1
    mflo $v0
    nop
    nop
    mult $v0, $t3
    mfhi $s0
    addu $v0, $s0, $v0
    j .Lcontrols_80172d7c
    srl $a2, $v0, 5
.Lcontrols_80172d4c:
    andi $v0, $t0, 0xffff
    subu $v1, $t6, $v1
    mult $v0, $v1
    mflo $v0
    nop
    nop
    mult $v0, $t3
    mfhi $s0
    addu $v1, $s0, $v0
    sra $v1, $v1, 5
    sra $v0, $v0, 0x1f
    subu $t0, $v1, $v0
.Lcontrols_80172d7c:
    sll $v0, $t2, 0x10
    sra $v0, $v0, 0x10
    sll $v1, $v0, 1
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    lui $at, %hi(audio_sequence_voice_globals + 170)
    addu $at, $at, $v1
    lbu $v1, %lo(audio_sequence_voice_globals + 170)($at)
    nop
    sltiu $v0, $v1, 0x40
    beqz $v0, .Lcontrols_80172dd8
    andi $v0, $a2, 0xffff
    mult $v0, $v1
    mflo $v0
    nop
    nop
    mult $v0, $t3
    mfhi $s0
    addu $v0, $s0, $v0
    j .Lcontrols_80172e08
    srl $a2, $v0, 5
.Lcontrols_80172dd8:
    andi $v0, $t0, 0xffff
    subu $v1, $t6, $v1
    mult $v0, $v1
    mflo $v0
    nop
    nop
    mult $v0, $t3
    mfhi $s0
    addu $v1, $s0, $v0
    sra $v1, $v1, 5
    sra $v0, $v0, 0x1f
    subu $t0, $v1, $v0
.Lcontrols_80172e08:
    lui $v1, %hi(audio_sequence_voice_control_e246)
    lh $v1, %lo(audio_sequence_voice_control_e246)($v1)
    addiu $v0, $zero, 1
    bne $v1, $v0, .Lcontrols_80172e3c
    andi $v0, $t0, 0xffff
    andi $v1, $a2, 0xffff
    sltu $v0, $v0, $v1
    beqz $v0, .Lcontrols_80172e34
    nop
    j .Lcontrols_80172e38
    addu $t0, $a2, $zero
.Lcontrols_80172e34:
    addu $a2, $t0, $zero
.Lcontrols_80172e38:
    andi $v0, $t0, 0xffff
.Lcontrols_80172e3c:
    mult $v0, $v0
    mflo $v0
    andi $a0, $a2, 0xffff
    nop
    mult $a0, $a0
    mflo $a0
    nop
    nop
    mult $v0, $t4
    mfhi $v1
    nop
    nop
    mult $a0, $t4
    addu $v1, $v1, $v0
    sra $v1, $v1, 0xd
    sra $v0, $v0, 0x1f
    subu $t0, $v1, $v0
    mfhi $a1
    addu $v0, $a1, $a0
    sra $v0, $v0, 0xd
    sra $a0, $a0, 0x1f
    subu $a2, $v0, $a0
    sll $a0, $t2, 0x10
    sra $a0, $a0, 0x10
    sll $v1, $a0, 4
    addu $v0, $v1, $t8
    addu $v1, $v1, $t9
    sh $t0, 0($v0)
    sh $a2, 0($v1)
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $a0
    lbu $v0, %lo(audio_sequence_voice_dirty)($at)
    nop
    ori $v0, $v0, 3
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $a0
    sb $v0, %lo(audio_sequence_voice_dirty)($at)
    addiu $v0, $t2, 1
.Lcontrols_80172ed4:
    addu $t2, $v0, $zero
    sll $v0, $v0, 0x10
    lui $v1, %hi(audio_sequence_voice_globals + 1812)
    lbu $v1, %lo(audio_sequence_voice_globals + 1812)($v1)
    sra $v0, $v0, 0x10
    slt $v0, $v0, $v1
    bnez $v0, .Lcontrols_80172ad4
    sll $v1, $t2, 0x10
.Lcontrols_80172ef4:
    lui $v0, %hi(audio_sequence_voice_owner)
    lh $v0, %lo(audio_sequence_voice_owner)($v0)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_set_gain

# exe/slus_004_22@0x80172F10..0x80172F7C
.globl audio_sequence_get_gain
.ent audio_sequence_get_gain
audio_sequence_get_gain:
    andi $v0, $a0, 0xff
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v0
    lw $v1, %lo(audio_sequence_handles)($at)
    lui $a3, %hi(audio_sequence_voice_owner)
    addiu $a3, $a3, %lo(audio_sequence_voice_owner)
    sh $a0, 0($a3)
    andi $a0, $a0, 0xff00
    sra $a0, $a0, 8
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    addu $v1, $v1, $v0
    lhu $v0, 0x74($v1)
    nop
    sh $v0, 0($a1)
    lhu $v0, 0x76($v1)
    nop
    sh $v0, 0($a2)
    lh $v0, 0($a3)
    jr $ra
    nop
.end audio_sequence_get_gain

# exe/slus_004_22@0x80172F7C..0x80172FCC
.globl audio_sequence_get_left_gain
.ent audio_sequence_get_left_gain
audio_sequence_get_left_gain:
    andi $v0, $a0, 0xff
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v0
    lw $v1, %lo(audio_sequence_handles)($at)
    lui $at, %hi(audio_sequence_voice_owner)
    sh $a0, %lo(audio_sequence_voice_owner)($at)
    andi $a0, $a0, 0xff00
    sra $a0, $a0, 8
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    addu $v1, $v1, $v0
    lh $v0, 0x74($v1)
    jr $ra
    nop
.end audio_sequence_get_left_gain

# exe/slus_004_22@0x80172FCC..0x8017301C
.globl audio_sequence_get_right_gain
.ent audio_sequence_get_right_gain
audio_sequence_get_right_gain:
    andi $v0, $a0, 0xff
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v0
    lw $v1, %lo(audio_sequence_handles)($at)
    lui $at, %hi(audio_sequence_voice_owner)
    sh $a0, %lo(audio_sequence_voice_owner)($at)
    andi $a0, $a0, 0xff00
    sra $a0, $a0, 8
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    addu $v1, $v1, $v0
    lh $v0, 0x76($v1)
    jr $ra
    nop
.end audio_sequence_get_right_gain

# exe/slus_004_22@0x8017301C..0x801730B8
.globl audio_sequence_release_owner_voices
.ent audio_sequence_release_owner_voices
audio_sequence_release_owner_voices:
    lui $v0, %hi(audio_sequence_voice_globals + 1812)
    lbu $v0, %lo(audio_sequence_voice_globals + 1812)($v0)
    addiu $sp, $sp, -0x20
    sw $s0, 0x10($sp)
    addu $s0, $zero, $zero
    sw $ra, 0x18($sp)
    beqz $v0, .Lcontrols_801730a0
    sw $s1, 0x14($sp)
    sll $v0, $a0, 0x10
    sra $s1, $v0, 0x10
    andi $v0, $s0, 0xff
.Lcontrols_80173048:
    sll $v1, $v0, 1
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    lui $at, %hi(audio_sequence_voice_globals + 174)
    addu $at, $at, $v1
    lh $v0, %lo(audio_sequence_voice_globals + 174)($at)
    nop
    bne $v0, $s1, .Lcontrols_80173084
    andi $v0, $s0, 0xff
    lui $at, %hi(audio_sequence_active_voice)
    sh $v0, %lo(audio_sequence_active_voice)($at)
    jal audio_sequence_release_voice
    addu $a0, $zero, $zero
.Lcontrols_80173084:
    addiu $s0, $s0, 1
    lui $v1, %hi(audio_sequence_voice_globals + 1812)
    lbu $v1, %lo(audio_sequence_voice_globals + 1812)($v1)
    andi $v0, $s0, 0xff
    sltu $v0, $v0, $v1
    bnez $v0, .Lcontrols_80173048
    andi $v0, $s0, 0xff
.Lcontrols_801730a0:
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_release_owner_voices

# exe/slus_004_22@0x801730B8..0x8017316C
.globl audio_sequence_select_tones
.ent audio_sequence_select_tones
audio_sequence_select_tones:
    addu $t1, $a0, $zero
    addu $a3, $zero, $zero
    lui $v1, %hi(audio_sequence_tone_selection)
    addiu $v1, $v1, %lo(audio_sequence_tone_selection)
    lbu $v0, 0($v1)
    nop
    beqz $v0, .Lcontrols_80173164
    addu $a2, $zero, $zero
    addiu $t0, $v1, 7
    addu $t2, $v1, $zero
.Lcontrols_801730e0:
    lbu $v0, 0($t0)
    andi $v1, $a2, 0xff
    sll $v0, $v0, 4
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_sequence_selected_tones)
    lw $v1, %lo(audio_sequence_selected_tones)($v1)
    sll $v0, $v0, 5
    addu $a0, $v0, $v1
    lbu $v0, 6($a0)
    lbu $v1, -5($t0)
    nop
    sltu $v0, $v1, $v0
    bnez $v0, .Lcontrols_8017314c
    nop
    lbu $v0, 7($a0)
    nop
    sltu $v0, $v0, $v1
    bnez $v0, .Lcontrols_8017314c
    andi $v0, $a3, 0xff
    addu $v1, $a3, $zero
    addiu $a3, $a3, 1
    addu $v0, $a1, $v0
    andi $v1, $v1, 0xff
    lhu $a0, 0x16($a0)
    addu $v1, $t1, $v1
    sb $a0, 0($v0)
    sb $a2, 0($v1)
.Lcontrols_8017314c:
    addiu $a2, $a2, 1
    lbu $v1, 0($t2)
    andi $v0, $a2, 0xff
    sltu $v0, $v0, $v1
    bnez $v0, .Lcontrols_801730e0
    nop
.Lcontrols_80173164:
    jr $ra
    andi $v0, $a3, 0xff
.end audio_sequence_select_tones

# exe/slus_004_22@0x8017316C..0x801736F0
.globl audio_sequence_set_program_gain
.ent audio_sequence_set_program_gain
audio_sequence_set_program_gain:
    addiu $sp, $sp, -0x30
    sw $s3, 0x1c($sp)
    addu $s3, $a3, $zero
    sw $s4, 0x20($sp)
    addu $s4, $zero, $zero
    sw $s5, 0x24($sp)
    addu $s5, $a0, $zero
    sw $s6, 0x28($sp)
    addu $s6, $a1, $zero
    andi $v1, $a0, 0xff
    sll $v1, $v1, 2
    andi $a0, $a0, 0xff00
    sra $a0, $a0, 8
    sll $v0, $a0, 1
    addu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    sll $a1, $a1, 0x10
    sll $a2, $a2, 0x10
    sw $s0, 0x10($sp)
    sra $s0, $a2, 0x10
    sra $a0, $a1, 0x10
    addu $a1, $s0, $zero
    sw $ra, 0x2c($sp)
    sw $s2, 0x18($sp)
    sw $s1, 0x14($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v1
    lw $v1, %lo(audio_sequence_handles)($at)
    lhu $s2, 0x40($sp)
    jal audio_sequence_select_bank_program
    addu $s1, $v1, $v0
    lui $at, %hi(audio_sequence_voice_owner)
    sh $s5, %lo(audio_sequence_voice_owner)($at)
    bnez $s2, .Lcontrols_8017320c
    andi $v0, $s3, 0xffff
    addiu $s2, $zero, 1
.Lcontrols_8017320c:
    bnez $v0, .Lcontrols_80173218
    nop
    addiu $s3, $zero, 1
.Lcontrols_80173218:
    lui $v0, %hi(audio_sequence_voice_globals + 1812)
    lbu $v0, %lo(audio_sequence_voice_globals + 1812)($v0)
    nop
    slt $v0, $s4, $v0
    beqz $v0, .Lcontrols_801736c0
    addu $t1, $zero, $zero
    addu $t5, $s0, $zero
    sll $s0, $t5, 4
    lui $t4, 0x204
    ori $t4, $t4, 0x811
    lui $t2, 0x410
    ori $t2, $t2, 0x4105
    addiu $t6, $zero, 0x7f
    lui $t3, 4
    ori $t3, $t3, 0x11
    lui $t7, %hi(audio_sequence_voice_register_cache)
    addiu $t7, $t7, %lo(audio_sequence_voice_register_cache)
    addiu $t8, $t7, 2
    sll $v1, $t1, 0x10
.Lcontrols_80173264:
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $a0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 174)
    addu $at, $at, $a0
    lh $v1, %lo(audio_sequence_voice_globals + 174)($at)
    sll $v0, $s5, 0x10
    sra $v0, $v0, 0x10
    bne $v1, $v0, .Lcontrols_801736a0
    addiu $v0, $t1, 1
    lui $at, %hi(audio_sequence_voice_globals + 178)
    addu $at, $at, $a0
    lh $v0, %lo(audio_sequence_voice_globals + 178)($at)
    nop
    bne $v0, $t5, .Lcontrols_801736a0
    addiu $v0, $t1, 1
    lui $at, %hi(audio_sequence_voice_globals + 182)
    addu $at, $at, $a0
    lh $v1, %lo(audio_sequence_voice_globals + 182)($at)
    sll $v0, $s6, 0x10
    sra $v0, $v0, 0x10
    bne $v1, $v0, .Lcontrols_801736a0
    addiu $v0, $t1, 1
    lbu $v0, 0x12($s1)
    nop
    sll $v0, $v0, 1
    addu $a0, $v0, $s1
    lh $v1, 0x4e($a0)
    andi $v0, $s3, 0xffff
    beq $v1, $v0, .Lcontrols_801732fc
    nop
    bnez $v1, .Lcontrols_80173300
    sll $v0, $t1, 0x10
    addiu $v0, $zero, 1
    sh $v0, 0x4e($a0)
.Lcontrols_801732fc:
    sll $v0, $t1, 0x10
.Lcontrols_80173300:
    sra $v0, $v0, 0x10
    sll $a3, $v0, 1
    addu $a3, $a3, $v0
    sll $a3, $a3, 2
    addu $a3, $a3, $v0
    sll $a3, $a3, 2
    lui $at, %hi(audio_sequence_voice_globals + 168)
    addu $at, $at, $a3
    lh $v1, %lo(audio_sequence_voice_globals + 168)($at)
    andi $v0, $s3, 0xffff
    mult $v1, $v0
    mflo $v1
    lui $v0, 0x8102
    ori $v0, $v0, 0x409
    mult $v1, $v0
    lui $v0, %hi(audio_sequence_selected_header)
    lw $v0, %lo(audio_sequence_selected_header)($v0)
    nop
    lbu $a1, 0x18($v0)
    nop
    sll $v0, $a1, 0xe
    subu $v0, $v0, $a1
    mfhi $t0
    addu $a0, $t0, $v1
    sra $a0, $a0, 6
    sra $v1, $v1, 0x1f
    subu $a0, $a0, $v1
    mult $a0, $v0
    mflo $a0
    lui $v0, 0x8206
    ori $v0, $v0, 0x1029
    mult $a0, $v0
    lui $v0, %hi(audio_sequence_selected_programs)
    lw $v0, %lo(audio_sequence_selected_programs)($v0)
    nop
    addu $v0, $s0, $v0
    lbu $v0, 1($v0)
    mfhi $v1
    addu $v1, $v1, $a0
    sra $v1, $v1, 0xd
    sra $a0, $a0, 0x1f
    subu $t0, $v1, $a0
    mult $t0, $v0
    lui $at, %hi(audio_sequence_voice_globals + 176)
    addu $at, $at, $a3
    lh $a2, %lo(audio_sequence_voice_globals + 176)($at)
    lui $at, %hi(audio_sequence_voice_globals + 180)
    addu $at, $at, $a3
    lh $v0, %lo(audio_sequence_voice_globals + 180)($at)
    sll $a2, $a2, 4
    addu $a2, $a2, $v0
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    sll $a2, $a2, 5
    addu $a2, $a2, $v0
    mflo $v1
    lbu $v0, 2($a2)
    nop
    mult $v1, $v0
    mflo $v0
    lui $v1, 0x40c
    ori $v1, $v1, 0x2051
    multu $v0, $v1
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    lhu $v0, 0x74($s1)
    srl $a0, $v1, 0xd
    mult $a0, $v0
    mflo $v0
    lhu $v1, 0x76($s1)
    nop
    mult $a0, $v1
    mflo $v1
    nop
    nop
    multu $v0, $t4
    mfhi $a0
    nop
    nop
    multu $v1, $t4
    subu $v0, $v0, $a0
    srl $v0, $v0, 1
    addu $a0, $a0, $v0
    srl $t0, $a0, 6
    mfhi $a1
    subu $v1, $v1, $a1
    srl $v1, $v1, 1
    addu $a1, $a1, $v1
    lbu $v1, 3($a2)
    nop
    sltiu $v0, $v1, 0x40
    beqz $v0, .Lcontrols_801734ac
    srl $a0, $a1, 6
    mult $a0, $v1
    mflo $v0
    nop
    nop
    multu $v0, $t2
    addu $a2, $t0, $zero
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    j .Lcontrols_801734dc
    srl $a1, $v1, 5
.Lcontrols_801734ac:
    subu $v0, $t6, $v1
    mult $t0, $v0
    mflo $v0
    nop
    nop
    multu $v0, $t2
    addu $a1, $a0, $zero
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $a2, $v1, 5
.Lcontrols_801734dc:
    sll $v1, $t1, 0x10
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 178)
    addu $at, $at, $v0
    lh $v0, %lo(audio_sequence_voice_globals + 178)($at)
    lui $v1, %hi(audio_sequence_selected_programs)
    lw $v1, %lo(audio_sequence_selected_programs)($v1)
    sll $v0, $v0, 4
    addu $v0, $v0, $v1
    lbu $v1, 4($v0)
    nop
    sltiu $v0, $v1, 0x40
    beqz $v0, .Lcontrols_80173550
    mult $a1, $v1
    mflo $v0
    nop
    nop
    multu $v0, $t2
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    j .Lcontrols_8017357c
    srl $a1, $v1, 5
.Lcontrols_80173550:
    subu $v0, $t6, $v1
    mult $a2, $v0
    mflo $v0
    nop
    nop
    multu $v0, $t2
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $a2, $v1, 5
.Lcontrols_8017357c:
    andi $v1, $s2, 0xff
    sltiu $v0, $v1, 0x40
    beqz $v0, .Lcontrols_801735b4
    mult $a1, $v1
    mflo $v0
    nop
    nop
    multu $v0, $t2
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    j .Lcontrols_801735e0
    srl $a1, $v1, 5
.Lcontrols_801735b4:
    subu $v0, $t6, $v1
    mult $a2, $v0
    mflo $v0
    nop
    nop
    multu $v0, $t2
    mfhi $v1
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $a2, $v1, 5
.Lcontrols_801735e0:
    lui $v1, %hi(audio_sequence_voice_control_e246)
    lh $v1, %lo(audio_sequence_voice_control_e246)($v1)
    addiu $v0, $zero, 1
    bne $v1, $v0, .Lcontrols_80173608
    sltu $v0, $a2, $a1
    beqz $v0, .Lcontrols_80173604
    nop
    j .Lcontrols_80173608
    addu $a2, $a1, $zero
.Lcontrols_80173604:
    addu $a1, $a2, $zero
.Lcontrols_80173608:
    mult $a2, $a2
    mflo $v0
    nop
    nop
    mult $a1, $a1
    mflo $a0
    nop
    nop
    multu $v0, $t3
    mfhi $v1
    nop
    nop
    multu $a0, $t3
    subu $v0, $v0, $v1
    srl $v0, $v0, 1
    addu $v1, $v1, $v0
    srl $a2, $v1, 0xd
    mfhi $a1
    subu $a0, $a0, $a1
    srl $a0, $a0, 1
    addu $a1, $a1, $a0
    srl $a1, $a1, 0xd
    sll $a0, $t1, 0x10
    sra $a0, $a0, 0x10
    sll $v1, $a0, 4
    addu $v0, $v1, $t7
    addu $v1, $v1, $t8
    sh $a2, 0($v0)
    sh $a1, 0($v1)
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $a0
    lbu $v0, %lo(audio_sequence_voice_dirty)($at)
    addiu $s4, $s4, 1
    ori $v0, $v0, 3
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $a0
    sb $v0, %lo(audio_sequence_voice_dirty)($at)
    addiu $v0, $t1, 1
.Lcontrols_801736a0:
    addu $t1, $v0, $zero
    sll $v0, $v0, 0x10
    lui $v1, %hi(audio_sequence_voice_globals + 1812)
    lbu $v1, %lo(audio_sequence_voice_globals + 1812)($v1)
    sra $v0, $v0, 0x10
    slt $v0, $v0, $v1
    bnez $v0, .Lcontrols_80173264
    sll $v1, $t1, 0x10
.Lcontrols_801736c0:
    addu $v0, $s4, $zero
    lw $ra, 0x2c($sp)
    lw $s6, 0x28($sp)
    lw $s5, 0x24($sp)
    lw $s4, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x30
    jr $ra
    nop
.end audio_sequence_set_program_gain

# exe/slus_004_22@0x801736F0..0x801737B4
.globl audio_sequence_select_bank_program
.ent audio_sequence_select_bank_program
audio_sequence_select_bank_program:
    addu $a3, $a0, $zero
    andi $v0, $a0, 0xffff
    sltiu $v0, $v0, 0x10
    beqz $v0, .Lcontrols_80173740
    addu $t0, $a1, $zero
    sll $v0, $a0, 0x10
    sra $a0, $v0, 0x10
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $a0
    lbu $v1, %lo(audio_sequence_voice_flags)($at)
    addiu $v0, $zero, 1
    bne $v1, $v0, .Lcontrols_801737ac
    addiu $v0, $zero, -1
    sll $v1, $a1, 0x10
    lui $v0, %hi(audio_sequence_voice_control_e24c)
    lh $v0, %lo(audio_sequence_voice_control_e24c)($v0)
    sra $v1, $v1, 0x10
    slt $v0, $v1, $v0
    bnez $v0, .Lcontrols_80173748
    sll $v0, $a0, 2
.Lcontrols_80173740:
    j .Lcontrols_801737ac
    addiu $v0, $zero, -1
.Lcontrols_80173748:
    lui $at, %hi(audio_sequence_bank_headers)
    addu $at, $at, $v0
    lw $a1, %lo(audio_sequence_bank_headers)($at)
    lui $at, %hi(audio_sequence_bank_programs)
    addu $at, $at, $v0
    lw $a0, %lo(audio_sequence_bank_programs)($at)
    lui $at, %hi(audio_sequence_bank_tones)
    addu $at, $at, $v0
    lw $a2, %lo(audio_sequence_bank_tones)($at)
    sll $v0, $v1, 4
    lui $at, %hi(audio_sequence_selected_bank)
    sb $a3, %lo(audio_sequence_selected_bank)($at)
    lui $at, %hi(audio_sequence_selected_program)
    sb $t0, %lo(audio_sequence_selected_program)($at)
    addu $v0, $v0, $a0
    lw $v1, 8($v0)
    addu $v0, $zero, $zero
    lui $at, %hi(audio_sequence_selected_header)
    sw $a1, %lo(audio_sequence_selected_header)($at)
    lui $at, %hi(audio_sequence_selected_programs)
    sw $a0, %lo(audio_sequence_selected_programs)($at)
    lui $at, %hi(audio_sequence_selected_tones)
    sw $a2, %lo(audio_sequence_selected_tones)($at)
    lui $at, %hi(audio_sequence_selected_tone_block)
    sb $v1, %lo(audio_sequence_selected_tone_block)($at)
.Lcontrols_801737ac:
    jr $ra
    nop
.end audio_sequence_select_bank_program
