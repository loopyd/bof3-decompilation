# SPDX-License-Identifier: MIT
# Owned SEP handle opening and sequence record initialization.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Handles and writable sequence records share the initialized sound state.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x8016B38C..0x8016B4B8
.globl audio_sequence_open
.ent audio_sequence_open
audio_sequence_open:
    addiu $sp, $sp, -0x38
    sw $s1, 0x1c($sp)
    addu $s1, $a0, $zero
    sw $s2, 0x20($sp)
    addu $s2, $zero, $zero
    lui $a3, %hi(audio_sequence_allocation_mask)
    lw $a3, %lo(audio_sequence_allocation_mask)($a3)
    addiu $v0, $zero, -1
    sw $ra, 0x30($sp)
    sw $s5, 0x2c($sp)
    sw $s4, 0x28($sp)
    sw $s3, 0x24($sp)
    bne $a3, $v0, .Lopening_8016b3dc
    sw $s0, 0x18($sp)
    lui $a0, %hi(audio_sequence_no_free_handle_message)
    addiu $a0, $a0, %lo(audio_sequence_no_free_handle_message)
    jal audio_spu_service_printf
    nop
.Lopening_8016b3d4:
    j .Lopening_8016b490
    addiu $v0, $zero, -1
.Lopening_8016b3dc:
    addu $v1, $zero, $zero
    addu $a0, $zero, $zero
    addiu $t0, $zero, 1
.Lopening_8016b3e8:
    sllv $v0, $t0, $v1
    and $v0, $a3, $v0
    bnez $v0, .Lopening_8016b404
    andi $v0, $a0, 0xff
    addu $s2, $v1, $zero
    addiu $a0, $zero, 1
    andi $v0, $a0, 0xff
.Lopening_8016b404:
    beqz $v0, .Lopening_8016b3e8
    addiu $v1, $v1, 1
    sll $a0, $s2, 0x10
    addiu $v0, $zero, 1
    lui $v1, %hi(audio_sequence_allocation_mask)
    lw $v1, %lo(audio_sequence_allocation_mask)($v1)
    sllv $v0, $v0, $s2
    or $v1, $v1, $v0
    sll $v0, $a2, 0x10
    sra $v0, $v0, 0x10
    lui $at, %hi(audio_sequence_allocation_mask)
    sw $v1, %lo(audio_sequence_allocation_mask)($at)
    blez $v0, .Lopening_8016b488
    addu $s0, $zero, $zero
    addu $s5, $a0, $zero
    sll $s4, $a1, 0x10
    addu $s3, $v0, $zero
.Lopening_8016b448:
    sra $a0, $s5, 0x10
    sll $a1, $s0, 0x10
    sra $a1, $a1, 0x10
    sra $a2, $s4, 0x10
    jal .Lopening_8016b4b8
    addu $a3, $s1, $zero
    addu $v1, $v0, $zero
    addiu $v0, $zero, -1
    beq $v1, $v0, .Lopening_8016b3d4
    addiu $v0, $s0, 1
    addu $s0, $v0, $zero
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slt $v0, $v0, $s3
    bnez $v0, .Lopening_8016b448
    addu $s1, $s1, $v1
.Lopening_8016b488:
    sll $v0, $s2, 0x10
    sra $v0, $v0, 0x10
.Lopening_8016b490:
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
.end audio_sequence_open

# exe/slus_004_22@0x8016B4B8..0x8016B868
.globl audio_sequence_open_record
.ent audio_sequence_open_record
audio_sequence_open_record:
.Lopening_8016b4b8:
    addiu $sp, $sp, -0x20
    addu $t4, $a0, $zero
    addu $t3, $a1, $zero
    sw $s1, 0x14($sp)
    addu $s1, $zero, $zero
    addu $t0, $zero, $zero
    addiu $t2, $zero, 0x40
    addiu $t1, $zero, 0x7f
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0xe
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
    sll $v0, $a1, 1
    addu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sw $ra, 0x1c($sp)
    sw $s2, 0x18($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a0
    lw $v1, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $s0, $v1, $v0
    addu $v1, $s0, $zero
    addiu $v0, $zero, 1
    sb $zero, 0x10($s0)
    sb $zero, 0x11($s0)
    sb $zero, 0x12($s0)
    sb $zero, 0x13($s0)
    sb $zero, 0x14($s0)
    sb $zero, 0x15($s0)
    sb $zero, 0x16($s0)
    sb $zero, 0x27($s0)
    sb $zero, 0x28($s0)
    sb $zero, 0x29($s0)
    sb $zero, 0x2a($s0)
    sb $zero, 0x2b($s0)
    sh $zero, 0x48($s0)
    sh $v0, 0x6e($s0)
    sh $zero, 0x4a($s0)
    sh $a2, 0x4c($s0)
    sh $zero, 0x72($s0)
    sw $zero, 0x7c($s0)
    sw $zero, 0x80($s0)
    sw $zero, 0x84($s0)
    sw $zero, 0x88($s0)
    sh $zero, 0xa8($s0)
.Lopening_8016b580:
    addu $v0, $s0, $t0
    sb $t2, 0x17($v0)
    sb $t0, 0x2c($v0)
    sh $t1, 0x4e($v1)
    addiu $t0, $t0, 1
    slti $v0, $t0, 0x10
    bnez $v0, .Lopening_8016b580
    addiu $v1, $v1, 2
    sll $v0, $t3, 0x10
    bnez $v0, .Lopening_8016b600
    sw $a3, 4($s0)
    lbu $v1, 0($a3)
    addiu $v0, $zero, 0x53
    beq $v1, $v0, .Lopening_8016b5c4
    addiu $v0, $zero, 0x70
    bne $v1, $v0, .Lopening_8016b610
    lui $a1, 0x393
.Lopening_8016b5c4:
    addiu $v0, $a3, 6
    sw $v0, 4($s0)
    lbu $v0, 5($a3)
    nop
    beqz $v0, .Lopening_8016b5f4
    addiu $v0, $a3, 8
    lui $a0, %hi(audio_sequence_invalid_sep_message)
    addiu $a0, $a0, %lo(audio_sequence_invalid_sep_message)
    jal audio_spu_service_printf
    nop
    j .Lopening_8016b84c
    addiu $v0, $zero, -1
.Lopening_8016b5f4:
    sw $v0, 4($s0)
    j .Lopening_8016b60c
    addiu $s1, $s1, 8
.Lopening_8016b600:
    addiu $v0, $a3, 2
    sw $v0, 4($s0)
    addiu $s1, $s1, 2
.Lopening_8016b60c:
    lui $a1, 0x393
.Lopening_8016b610:
    lw $v1, 4($s0)
    ori $a1, $a1, 0x8700
    addiu $v0, $v1, 1
    sw $v0, 4($s0)
    lbu $a2, 0($v1)
    addiu $v0, $v1, 2
    sw $v0, 4($s0)
    lbu $v0, 1($v1)
    lw $v1, 4($s0)
    sll $a0, $a2, 8
    or $v0, $v0, $a0
    sh $v0, 0x4a($s0)
    addiu $v0, $v1, 1
    sw $v0, 4($s0)
    lbu $a3, 0($v1)
    addiu $v0, $v1, 2
    sw $v0, 4($s0)
    lbu $a0, 1($v1)
    addiu $v0, $v1, 3
    sw $v0, 4($s0)
    lbu $v1, 2($v1)
    sll $v0, $a3, 0x10
    sll $a0, $a0, 8
    or $v0, $v0, $a0
    or $v0, $v0, $v1
    div $zero, $a1, $v0
    bnez $v0, .Lopening_8016b684
    nop
    break 7
.Lopening_8016b684:
    addiu $at, $zero, -1
    bne $v0, $at, .Lopening_8016b69c
    lui $at, 0x8000
    bne $a1, $at, .Lopening_8016b69c
    nop
    break 6
.Lopening_8016b69c:
    mflo $a1
    mfhi $v1
    sw $v0, 0x84($s0)
    srl $v0, $v0, 1
    slt $v0, $v0, $v1
    beqz $v0, .Lopening_8016b6c4
    addiu $s1, $s1, 5
    addiu $v0, $a1, 1
    j .Lopening_8016b6c8
    sw $v0, 0x84($s0)
.Lopening_8016b6c4:
    sw $a1, 0x84($s0)
.Lopening_8016b6c8:
    sll $a0, $t4, 0x10
    lw $v0, 0x84($s0)
    lw $a1, 4($s0)
    sra $a0, $a0, 0x10
    sw $v0, 0x8c($s0)
    addiu $v0, $a1, 3
    sw $v0, 4($s0)
    lbu $a2, 2($a1)
    addiu $v0, $a1, 4
    sw $v0, 4($s0)
    lbu $v1, 3($a1)
    addiu $v0, $a1, 5
    sw $v0, 4($s0)
    lbu $a3, 4($a1)
    addiu $v0, $a1, 6
    sw $v0, 4($s0)
    lbu $a1, 5($a1)
    sll $v0, $a2, 0x18
    sll $v1, $v1, 0x10
    addu $v0, $v0, $v1
    sll $v1, $a3, 8
    addu $v0, $v0, $v1
    addu $s2, $v0, $a1
    sll $a1, $t3, 0x10
    jal audio_sequence_read_delta
    sra $a1, $a1, 0x10
    lh $a0, 0x4a($s0)
    lw $v1, 0x84($s0)
    nop
    mult $a0, $v1
    lw $a0, 4($s0)
    lw $v1, 4($s0)
    sw $a0, 0xc($s0)
    lui $a0, %hi(audio_sequence_tick_rate)
    lw $a0, %lo(audio_sequence_tick_rate)($a0)
    sw $v0, 0x7c($s0)
    sw $v0, 0x88($s0)
    sw $v1, 8($s0)
    sll $v1, $a0, 4
    subu $a3, $v1, $a0
    sll $a1, $a3, 2
    mflo $a2
    sll $v0, $a2, 2
    addu $v0, $v0, $a2
    sll $v0, $v0, 1
    sltu $v0, $v0, $a1
    beqz $v0, .Lopening_8016b7b8
    addiu $s1, $s1, 6
    sll $v1, $a0, 2
    addu $v1, $v1, $a0
    sll $v0, $v1, 4
    subu $v0, $v0, $v1
    sll $v0, $v0, 3
    divu $zero, $v0, $a2
    bnez $a2, .Lopening_8016b7ac
    nop
    break 7
.Lopening_8016b7ac:
    mflo $v0
    j .Lopening_8016b83c
    sh $v0, 0x6e($s0)
.Lopening_8016b7b8:
    lh $v1, 0x4a($s0)
    lw $v0, 0x84($s0)
    nop
    mult $v1, $v0
    mflo $v1
    sll $v0, $v1, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 1
    divu $zero, $v0, $a1
    bnez $a1, .Lopening_8016b7e8
    nop
    break 7
.Lopening_8016b7e8:
    mflo $a0
    lh $v1, 0x4a($s0)
    lw $v0, 0x84($s0)
    nop
    mult $v1, $v0
    mflo $v1
    sll $v0, $v1, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 1
    divu $zero, $v0, $a1
    bnez $a1, .Lopening_8016b81c
    nop
    break 7
.Lopening_8016b81c:
    mfhi $v1
    addiu $v0, $zero, -1
    sh $v0, 0x6e($s0)
    sll $v0, $a3, 1
    sltu $v0, $v0, $v1
    beqz $v0, .Lopening_8016b840
    sh $a0, 0x70($s0)
    addiu $v0, $a0, 1
.Lopening_8016b83c:
    sh $v0, 0x70($s0)
.Lopening_8016b840:
    lhu $v1, 0x70($s0)
    addu $v0, $s1, $s2
    sh $v1, 0x72($s0)
.Lopening_8016b84c:
    lw $ra, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_open_record

.section .data
.balign 4
# exe/slus_004_22@0x8014A048, 36 bytes.
.globl audio_sequence_no_free_handle_message
audio_sequence_no_free_handle_message:
    .byte 0x43, 0x61, 0x6e, 0x27, 0x74, 0x20, 0x4f, 0x70, 0x65, 0x6e, 0x20, 0x53, 0x65, 0x71, 0x75, 0x65, 0x6e, 0x63, 0x65, 0x20, 0x64, 0x61, 0x74, 0x61, 0x20, 0x61, 0x6e, 0x79, 0x20, 0x6d, 0x6f, 0x72, 0x65, 0x0a, 0x0a, 0x00
# exe/slus_004_22@0x8014A06C, 23 bytes.
.globl audio_sequence_invalid_sep_message
audio_sequence_invalid_sep_message:
    .byte 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x53, 0x45, 0x50, 0x20, 0x44, 0x61, 0x74, 0x61, 0x2e, 0x0a, 0x00

# exe/slus_004_22 playback entry and mode setter.
.section .text
.balign 4

# exe/slus_004_22@0x8016B9CC..0x8016BA04
.globl audio_sequence_play
.ent audio_sequence_play
audio_sequence_play:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    sll $a0, $a0, 0x10
    sll $a1, $a1, 0x10
    sll $a3, $a3, 0x10
    sra $a0, $a0, 0x10
    sra $a1, $a1, 0x10
    andi $a2, $a2, 0xff
    jal .Lplay_8016bb98
    sra $a3, $a3, 0x10
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_play

# exe/slus_004_22@0x8016BB98..0x8016BC98
.globl audio_sequence_set_play_mode
.ent audio_sequence_set_play_mode
audio_sequence_set_play_mode:
.Lplay_8016bb98:
    addiu $sp, $sp, -0x18
    addu $t3, $a0, $zero
    sll $v0, $t3, 0x10
    lui $v1, %hi(audio_sequence_handles)
    addiu $v1, $v1, %lo(audio_sequence_handles)
    sra $v0, $v0, 0xe
    addu $t2, $v0, $v1
    sll $v1, $a1, 0x10
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sw $ra, 0x10($sp)
    lw $v1, 0($t2)
    sll $t1, $v0, 2
    addu $t0, $v1, $t1
    lw $v0, 0x90($t0)
    addiu $v1, $zero, -0x201
    and $v0, $v0, $v1
    sw $v0, 0x90($t0)
    lw $v1, 0($t2)
    addiu $a0, $zero, -5
    addu $v1, $t1, $v1
    lw $v0, 0x90($v1)
    andi $a2, $a2, 0xff
    and $v0, $v0, $a0
    sw $v0, 0x90($v1)
    addiu $v0, $zero, 1
    bne $a2, $v0, .Lplay_8016bc64
    sh $a3, 0x46($t0)
    sll $a0, $a1, 8
    or $a0, $t3, $a0
    sll $a0, $a0, 0x10
    lw $v1, 0($t2)
    sra $a0, $a0, 0x10
    addu $v1, $t1, $v1
    lw $v0, 0x90($v1)
    addu $a3, $zero, $zero
    ori $v0, $v0, 1
    sw $v0, 0x90($v1)
    lhu $a1, 0x74($t0)
    lhu $a2, 0x76($t0)
    addiu $v0, $zero, 1
    sh $zero, 0x48($t0)
    jal audio_sequence_set_gain
    sb $v0, 0x2b($t0)
    j .Lplay_8016bc88
    nop
.Lplay_8016bc64:
    bnez $a2, .Lplay_8016bc88
    nop
    lw $v1, 0($t2)
    nop
    addu $v1, $t1, $v1
    lw $v0, 0x90($v1)
    nop
    ori $v0, $v0, 2
    sw $v0, 0x90($v1)
.Lplay_8016bc88:
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_set_play_mode
