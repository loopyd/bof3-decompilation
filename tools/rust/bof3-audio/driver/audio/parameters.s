# SPDX-License-Identifier: MIT
# Owned sequence data entry and program/tone attribute copies.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Preserve live selection, copy order, untouched fields and callback stack ABI.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x80169AB8..0x80169EE8
.globl audio_sequence_data_entry_command
.ent audio_sequence_data_entry_command
audio_sequence_data_entry_command:
    addiu $sp, $sp, -0xd8
    addu $a3, $a0, $zero
    addu $t0, $a1, $zero
    sw $s4, 0xc0($sp)
    addu $s4, $a2, $zero
    addiu $a2, $sp, 0x38
    sll $v0, $a3, 0x10
    sw $s3, 0xbc($sp)
    sra $s3, $v0, 0x10
    sll $v1, $s3, 2
    sll $v0, $t0, 0x10
    sw $s1, 0xb4($sp)
    sra $s1, $v0, 0x10
    sll $v0, $s1, 1
    addu $v0, $v0, $s1
    sll $v0, $v0, 2
    subu $v0, $v0, $s1
    sll $v0, $v0, 2
    subu $v0, $v0, $s1
    sll $v0, $v0, 2
    sw $ra, 0xd4($sp)
    sw $fp, 0xd0($sp)
    sw $s7, 0xcc($sp)
    sw $s6, 0xc8($sp)
    sw $s5, 0xc4($sp)
    sw $s2, 0xb8($sp)
    sw $s0, 0xb0($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v1
    lw $v1, %lo(audio_sequence_handles)($at)
    addu $s7, $a3, $zero
    addu $s0, $v1, $v0
    lbu $s2, 0x12($s0)
    lh $a0, 0x4c($s0)
    addu $v0, $s2, $s0
    lbu $a1, 0x2c($v0)
    jal audio_sequence_get_program_attributes
    addu $fp, $t0, $zero
    lbu $v1, 0x27($s0)
    addiu $v0, $zero, 1
    bne $v1, $v0, .Lparameters_80169b88
    nop
    lbu $v0, 0x10($s0)
    nop
    bnez $v0, .Lparameters_80169b88
    addu $a0, $s3, $zero
    addu $a1, $s1, $zero
    addiu $v0, $zero, 1
    sb $s4, 0x28($s0)
    sb $zero, 0x27($s0)
    j .Lparameters_80169ea8
    sb $v0, 0x10($s0)
.Lparameters_80169b88:
    lbu $v0, 0x29($s0)
    addiu $v1, $zero, 2
    bne $v0, $v1, .Lparameters_80169ccc
    nop
    lbu $v0, 0x14($s0)
    nop
    bnez $v0, .Lparameters_80169cb0
    sll $a0, $s7, 0x10
    lbu $v0, 0x38($sp)
    nop
    blez $v0, .Lparameters_80169cb0
    addu $s1, $zero, $zero
    addu $s6, $s2, $s0
    andi $s3, $s4, 0xff
    sll $v0, $s3, 1
    addu $v0, $v0, $s3
    sll $v0, $v0, 3
    addu $s5, $v0, $s3
    sll $t1, $s5, 8
    sw $t1, 0x68($sp)
    addu $s2, $zero, $zero
.Lparameters_80169bdc:
    sra $a2, $s2, 0x10
    lh $a0, 0x4c($s0)
    lbu $a1, 0x2c($s6)
    jal audio_sequence_get_tone_attributes
    addiu $a3, $sp, 0x48
    lbu $v1, 0x13($s0)
    addiu $v0, $zero, 1
    beq $v1, $v0, .Lparameters_80169c38
    slti $v0, $v1, 2
    beqz $v0, .Lparameters_80169c18
    nop
    beqz $v1, .Lparameters_80169c2c
    andi $v0, $s4, 0x7f
    j .Lparameters_80169c80
    sra $a2, $s2, 0x10
.Lparameters_80169c18:
    addiu $v0, $zero, 2
    beq $v1, $v0, .Lparameters_80169c70
    sra $a2, $s2, 0x10
    j .Lparameters_80169c80
    nop
.Lparameters_80169c2c:
    sb $v0, 0x55($sp)
    j .Lparameters_80169c7c
    sb $v0, 0x54($sp)
.Lparameters_80169c38:
    addiu $v0, $s4, -0x41
    andi $v0, $v0, 0xff
    sltiu $v0, $v0, 0x3f
    beqz $v0, .Lparameters_80169c60
    sll $v0, $s5, 2
    bgez $v0, .Lparameters_80169c64
    andi $v1, $v0, 0xe000
    addiu $v0, $v0, 0x1fff
    j .Lparameters_80169c64
    andi $v1, $v0, 0xe000
.Lparameters_80169c60:
    addu $v1, $zero, $zero
.Lparameters_80169c64:
    lbu $v0, 0x4d($sp)
    j .Lparameters_80169c7c
    sb $v0, 0x4d($sp)
.Lparameters_80169c70:
    lbu $v0, 0x4c($sp)
    nop
    sb $v0, 0x4c($sp)
.Lparameters_80169c7c:
    sra $a2, $s2, 0x10
.Lparameters_80169c80:
    addiu $a3, $sp, 0x48
    lh $a0, 0x4c($s0)
    lbu $a1, 0x2c($s6)
    jal audio_sequence_set_tone_attributes
    addiu $s1, $s1, 1
    lui $v1, 1
    lbu $v0, 0x38($sp)
    nop
    slt $v0, $s1, $v0
    bnez $v0, .Lparameters_80169bdc
    addu $s2, $s2, $v1
    sll $a0, $s7, 0x10
.Lparameters_80169cb0:
    sra $a0, $a0, 0x10
    sll $a1, $fp, 0x10
    jal audio_sequence_read_delta
    sra $a1, $a1, 0x10
    sw $v0, 0x88($s0)
    j .Lparameters_80169eb4
    sb $zero, 0x29($s0)
.Lparameters_80169ccc:
    lbu $v0, 0x2a($s0)
    nop
    bne $v0, $v1, .Lparameters_80169e9c
    sll $a0, $s7, 0x10
    lbu $a2, 0x16($s0)
    addiu $v0, $zero, 0x10
    bne $a2, $v0, .Lparameters_80169dd0
    addu $v0, $s2, $s0
    lbu $v0, 0x38($sp)
    nop
    blez $v0, .Lparameters_80169e80
    addu $s1, $zero, $zero
    lui $s3, %hi(audio_sequence_parameter_callbacks)
    addiu $s3, $s3, %lo(audio_sequence_parameter_callbacks)
    addu $s2, $s2, $s0
    addu $a2, $s1, $zero
.Lparameters_80169d0c:
    lbu $v1, 0x15($s0)
    lh $a0, 0x4c($s0)
    lbu $a1, 0x2c($s2)
    andi $v0, $s4, 0xff
    sw $v0, 0x30($sp)
    sw $v1, 0x2c($sp)
    lhu $t3, 0x48($sp)
    lhu $a3, 0x4a($sp)
    lwl $v0, 0x4f($sp)
    lwr $v0, 0x4c($sp)
    lwl $t0, 0x53($sp)
    lwr $t0, 0x50($sp)
    lwl $t1, 0x57($sp)
    lwr $t1, 0x54($sp)
    lwl $t2, 0x5b($sp)
    lwr $t2, 0x58($sp)
    swl $v0, 0x13($sp)
    swr $v0, 0x10($sp)
    swl $t0, 0x17($sp)
    swr $t0, 0x14($sp)
    swl $t1, 0x1b($sp)
    swr $t1, 0x18($sp)
    swl $t2, 0x1f($sp)
    swr $t2, 0x1c($sp)
    lwl $v0, 0x5f($sp)
    lwr $v0, 0x5c($sp)
    lwl $t0, 0x63($sp)
    lwr $t0, 0x60($sp)
    lwl $t1, 0x67($sp)
    lwr $t1, 0x64($sp)
    swl $v0, 0x23($sp)
    swr $v0, 0x20($sp)
    swl $t0, 0x27($sp)
    swr $t0, 0x24($sp)
    swl $t1, 0x2b($sp)
    swr $t1, 0x28($sp)
    sll $v1, $v1, 2
    addu $v1, $v1, $s3
    lw $v0, 0($v1)
    sll $a3, $a3, 0x10
    jalr $v0
    or $a3, $t3, $a3
    lbu $v0, 0x38($sp)
    addiu $s1, $s1, 1
    slt $v0, $s1, $v0
    bnez $v0, .Lparameters_80169d0c
    addu $a2, $s1, $zero
    j .Lparameters_80169e80
    sll $a0, $s7, 0x10
.Lparameters_80169dd0:
    lbu $v1, 0x15($s0)
    lh $a0, 0x4c($s0)
    lbu $a1, 0x2c($v0)
    andi $v0, $s4, 0xff
    sw $v0, 0x30($sp)
    sw $v1, 0x2c($sp)
    lhu $t3, 0x48($sp)
    lhu $a3, 0x4a($sp)
    lwl $v0, 0x4f($sp)
    lwr $v0, 0x4c($sp)
    lwl $t0, 0x53($sp)
    lwr $t0, 0x50($sp)
    lwl $t1, 0x57($sp)
    lwr $t1, 0x54($sp)
    lwl $t2, 0x5b($sp)
    lwr $t2, 0x58($sp)
    swl $v0, 0x13($sp)
    swr $v0, 0x10($sp)
    swl $t0, 0x17($sp)
    swr $t0, 0x14($sp)
    swl $t1, 0x1b($sp)
    swr $t1, 0x18($sp)
    swl $t2, 0x1f($sp)
    swr $t2, 0x1c($sp)
    lwl $v0, 0x5f($sp)
    lwr $v0, 0x5c($sp)
    lwl $t0, 0x63($sp)
    lwr $t0, 0x60($sp)
    lwl $t1, 0x67($sp)
    lwr $t1, 0x64($sp)
    swl $v0, 0x23($sp)
    swr $v0, 0x20($sp)
    swl $t0, 0x27($sp)
    swr $t0, 0x24($sp)
    swl $t1, 0x2b($sp)
    swr $t1, 0x28($sp)
    sll $v1, $v1, 2
    lui $at, %hi(audio_sequence_parameter_callbacks)
    addu $at, $at, $v1
    lw $v0, %lo(audio_sequence_parameter_callbacks)($at)
    sll $a3, $a3, 0x10
    jalr $v0
    or $a3, $t3, $a3
    sll $a0, $s7, 0x10
.Lparameters_80169e80:
    sra $a0, $a0, 0x10
    sll $a1, $fp, 0x10
    jal audio_sequence_read_delta
    sra $a1, $a1, 0x10
    sw $v0, 0x88($s0)
    j .Lparameters_80169eb4
    sb $zero, 0x2a($s0)
.Lparameters_80169e9c:
    sra $a0, $a0, 0x10
    sll $a1, $fp, 0x10
    sra $a1, $a1, 0x10
.Lparameters_80169ea8:
    jal audio_sequence_read_delta
    nop
    sw $v0, 0x88($s0)
.Lparameters_80169eb4:
    lw $ra, 0xd4($sp)
    lw $fp, 0xd0($sp)
    lw $s7, 0xcc($sp)
    lw $s6, 0xc8($sp)
    lw $s5, 0xc4($sp)
    lw $s4, 0xc0($sp)
    lw $s3, 0xbc($sp)
    lw $s2, 0xb8($sp)
    lw $s1, 0xb4($sp)
    lw $s0, 0xb0($sp)
    addiu $sp, $sp, 0xd8
    jr $ra
    nop
.end audio_sequence_data_entry_command

# exe/slus_004_22@0x8016E0BC..0x8016E1C4
.globl audio_sequence_get_program_attributes
.ent audio_sequence_get_program_attributes
audio_sequence_get_program_attributes:
    addiu $sp, $sp, -0x20
    sw $s1, 0x14($sp)
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    sw $ra, 0x18($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $a0
    lbu $v1, %lo(audio_sequence_voice_flags)($at)
    addiu $v0, $zero, 1
    beq $v1, $v0, .Lparameters_8016e0f4
    addu $s1, $a2, $zero
    j .Lparameters_8016e1ac
    addiu $v0, $zero, -1
.Lparameters_8016e0f4:
    sll $s0, $a1, 0x10
    sra $s0, $s0, 0x10
    jal audio_sequence_select_bank_program
    addu $a1, $s0, $zero
    lui $v0, %hi(audio_sequence_selected_programs)
    lw $v0, %lo(audio_sequence_selected_programs)($v0)
    sll $s0, $s0, 4
    addu $v0, $s0, $v0
    lbu $v0, 0($v0)
    nop
    sb $v0, 0($s1)
    lui $v0, %hi(audio_sequence_selected_programs)
    lw $v0, %lo(audio_sequence_selected_programs)($v0)
    nop
    addu $v0, $s0, $v0
    lbu $v0, 1($v0)
    nop
    sb $v0, 1($s1)
    lui $v0, %hi(audio_sequence_selected_programs)
    lw $v0, %lo(audio_sequence_selected_programs)($v0)
    nop
    addu $v0, $s0, $v0
    lbu $v0, 2($v0)
    nop
    sb $v0, 2($s1)
    lui $v0, %hi(audio_sequence_selected_programs)
    lw $v0, %lo(audio_sequence_selected_programs)($v0)
    nop
    addu $v0, $s0, $v0
    lbu $v0, 3($v0)
    nop
    sb $v0, 3($s1)
    lui $v0, %hi(audio_sequence_selected_programs)
    lw $v0, %lo(audio_sequence_selected_programs)($v0)
    nop
    addu $v0, $s0, $v0
    lbu $v0, 4($v0)
    nop
    sb $v0, 4($s1)
    lui $v0, %hi(audio_sequence_selected_programs)
    lw $v0, %lo(audio_sequence_selected_programs)($v0)
    nop
    addu $s0, $s0, $v0
    lhu $v1, 6($s0)
    addu $v0, $zero, $zero
    sh $v1, 6($s1)
.Lparameters_8016e1ac:
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_get_program_attributes

# exe/slus_004_22@0x8016E1C4..0x8016E400
.globl audio_sequence_get_tone_attributes
.ent audio_sequence_get_tone_attributes
audio_sequence_get_tone_attributes:
    addiu $sp, $sp, -0x20
    sw $s1, 0x14($sp)
    addu $s1, $a2, $zero
    sw $s0, 0x10($sp)
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    sw $ra, 0x18($sp)
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $a0
    lbu $v1, %lo(audio_sequence_voice_flags)($at)
    addiu $v0, $zero, 1
    beq $v1, $v0, .Lparameters_8016e200
    addu $s0, $a3, $zero
    j .Lparameters_8016e3e8
    addiu $v0, $zero, -1
.Lparameters_8016e200:
    sll $a1, $a1, 0x10
    jal audio_sequence_select_bank_program
    sra $a1, $a1, 0x10
    lui $v1, %hi(audio_sequence_selected_tone_block)
    lbu $v1, %lo(audio_sequence_selected_tone_block)($v1)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    sll $v1, $v1, 4
    addu $v1, $s1, $v1
    sll $v1, $v1, 0x10
    sra $v1, $v1, 0xb
    addu $v0, $v1, $v0
    lbu $v0, 0($v0)
    nop
    sb $v0, 0($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 1($v0)
    nop
    sb $v0, 1($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 2($v0)
    nop
    sb $v0, 2($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 3($v0)
    nop
    sb $v0, 3($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 4($v0)
    nop
    sb $v0, 4($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 5($v0)
    nop
    sb $v0, 5($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 7($v0)
    nop
    sb $v0, 7($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 6($v0)
    nop
    sb $v0, 6($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 8($v0)
    nop
    sb $v0, 8($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 9($v0)
    nop
    sb $v0, 9($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 0xa($v0)
    nop
    sb $v0, 0xa($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 0xb($v0)
    nop
    sb $v0, 0xb($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 0xc($v0)
    nop
    sb $v0, 0xc($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v0, $v1, $v0
    lbu $v0, 0xd($v0)
    nop
    sb $v0, 0xd($s0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    nop
    addu $v1, $v1, $v0
    lhu $v0, 0x10($v1)
    nop
    sh $v0, 0x10($s0)
    lhu $v0, 0x12($v1)
    nop
    sh $v0, 0x12($s0)
    lhu $v0, 0x14($v1)
    nop
    sh $v0, 0x14($s0)
    lhu $v1, 0x16($v1)
    addu $v0, $zero, $zero
    sh $v1, 0x16($s0)
.Lparameters_8016e3e8:
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_get_tone_attributes

# exe/slus_004_22@0x8016F6D8..0x8016F8A0
.globl audio_sequence_set_tone_attributes
.ent audio_sequence_set_tone_attributes
audio_sequence_set_tone_attributes:
    addiu $sp, $sp, -0x20
    sw $s1, 0x14($sp)
    addu $s1, $a2, $zero
    sw $s0, 0x10($sp)
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    sw $ra, 0x18($sp)
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $a0
    lbu $v1, %lo(audio_sequence_voice_flags)($at)
    addiu $v0, $zero, 1
    beq $v1, $v0, .Lparameters_8016f714
    addu $s0, $a3, $zero
    j .Lparameters_8016f888
    addiu $v0, $zero, -1
.Lparameters_8016f714:
    sll $a1, $a1, 0x10
    jal audio_sequence_select_bank_program
    sra $a1, $a1, 0x10
    lui $v1, %hi(audio_sequence_selected_tone_block)
    lbu $v1, %lo(audio_sequence_selected_tone_block)($v1)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 0($s0)
    sll $v1, $v1, 4
    addu $v1, $s1, $v1
    sll $v1, $v1, 0x10
    sra $v1, $v1, 0xb
    addu $v0, $v1, $v0
    sb $a0, 0($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 1($s0)
    addu $v0, $v1, $v0
    sb $a0, 1($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 2($s0)
    addu $v0, $v1, $v0
    sb $a0, 2($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 3($s0)
    addu $v0, $v1, $v0
    sb $a0, 3($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 4($s0)
    addu $v0, $v1, $v0
    sb $a0, 4($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 5($s0)
    addu $v0, $v1, $v0
    sb $a0, 5($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 7($s0)
    addu $v0, $v1, $v0
    sb $a0, 7($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 6($s0)
    addu $v0, $v1, $v0
    sb $a0, 6($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 8($s0)
    addu $v0, $v1, $v0
    sb $a0, 8($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 9($s0)
    addu $v0, $v1, $v0
    sb $a0, 9($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 0xa($s0)
    addu $v0, $v1, $v0
    sb $a0, 0xa($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 0xb($s0)
    addu $v0, $v1, $v0
    sb $a0, 0xb($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 0xc($s0)
    addu $v0, $v1, $v0
    sb $a0, 0xc($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lbu $a0, 0xd($s0)
    addu $v0, $v1, $v0
    sb $a0, 0xd($v0)
    lui $v0, %hi(audio_sequence_selected_tones)
    lw $v0, %lo(audio_sequence_selected_tones)($v0)
    lhu $a0, 0x10($s0)
    addu $v1, $v1, $v0
    sh $a0, 0x10($v1)
    lhu $v0, 0x12($s0)
    nop
    sh $v0, 0x12($v1)
    lhu $v0, 0x14($s0)
    nop
    sh $v0, 0x14($v1)
    lhu $a0, 0x16($s0)
    addu $v0, $zero, $zero
    sh $a0, 0x16($v1)
.Lparameters_8016f888:
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_set_tone_attributes
