# SPDX-License-Identifier: MIT
# Owned sequence commands, live callbacks and event delta reading.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Callers own valid sequence records, event bytes and callback bindings.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x8016A090..0x8016A140
.globl audio_sequence_reverb_command
.ent audio_sequence_reverb_command
audio_sequence_reverb_command:
    addiu $sp, $sp, -0x20
    sw $s1, 0x14($sp)
    addu $s1, $a0, $zero
    sw $s2, 0x18($sp)
    addu $s2, $a1, $zero
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
    sll $v0, $v0, 2
    andi $a2, $a2, 0xff
    sw $ra, 0x1c($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a0
    lw $v1, %lo(audio_sequence_handles)($at)
    sltiu $a2, $a2, 0x40
    beqz $a2, .Lcommands_8016a104
    addu $s0, $v1, $v0
    jal audio_sequence_clear_reverb_state
    nop
    j .Lcommands_8016a110
    sll $a0, $s1, 0x10
.Lcommands_8016a104:
    jal audio_sequence_set_reverb_state
    nop
    sll $a0, $s1, 0x10
.Lcommands_8016a110:
    sll $a1, $s2, 0x10
    sra $a0, $a0, 0x10
    jal audio_sequence_read_delta
    sra $a1, $a1, 0x10
    sw $v0, 0x88($s0)
    lw $ra, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_reverb_command

# exe/slus_004_22@0x8016A140..0x8016A278
.globl audio_sequence_data_command
.ent audio_sequence_data_command
audio_sequence_data_command:
    addiu $sp, $sp, -0x20
    sw $s1, 0x14($sp)
    addu $s1, $a0, $zero
    sw $s2, 0x18($sp)
    addu $s2, $a1, $zero
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
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a0
    lw $v1, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $s0, $v1, $v0
    lbu $v1, 0x27($s0)
    addiu $v0, $zero, 1
    bne $v1, $v0, .Lcommands_8016a1c8
    nop
    lbu $v0, 0x10($s0)
    nop
    bnez $v0, .Lcommands_8016a1c8
    addiu $v0, $zero, 1
    sb $a2, 0x28($s0)
    sb $zero, 0x27($s0)
    j .Lcommands_8016a1f4
    sb $v0, 0x10($s0)
.Lcommands_8016a1c8:
    lbu $v1, 0x16($s0)
    addiu $v0, $zero, 0x1e
    beq $v1, $v0, .Lcommands_8016a1f8
    addiu $v0, $zero, 0x14
    beq $v1, $v0, .Lcommands_8016a1fc
    addiu $v0, $zero, 0x28
    lbu $v0, 0x2a($s0)
    sb $a2, 0x15($s0)
    sb $zero, 0x27($s0)
    addiu $v0, $v0, 1
    sb $v0, 0x2a($s0)
.Lcommands_8016a1f4:
    lbu $v1, 0x16($s0)
.Lcommands_8016a1f8:
    addiu $v0, $zero, 0x28
.Lcommands_8016a1fc:
    bne $v1, $v0, .Lcommands_8016a248
    sll $a0, $s1, 0x10
    sll $v0, $s1, 0x10
    sra $a0, $v0, 0x10
    lui $v0, %hi(audio_sequence_channel_matrix)
    addiu $v0, $v0, %lo(audio_sequence_channel_matrix)
    sll $v1, $a0, 6
    addu $v1, $v1, $v0
    sll $v0, $s2, 0x10
    sra $a1, $v0, 0x10
    sll $v0, $a1, 2
    addu $v0, $v0, $v1
    lw $v0, 0($v0)
    nop
    beqz $v0, .Lcommands_8016a244
    nop
    jalr $v0
    andi $a2, $a2, 0xff
.Lcommands_8016a244:
    sll $a0, $s1, 0x10
.Lcommands_8016a248:
    sra $a0, $a0, 0x10
    sll $a1, $s2, 0x10
    jal audio_sequence_read_delta
    sra $a1, $a1, 0x10
    sw $v0, 0x88($s0)
    lw $ra, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_data_command

# exe/slus_004_22@0x8016A278..0x8016A3B4
.globl audio_sequence_loop_command
.ent audio_sequence_loop_command
audio_sequence_loop_command:
    addiu $sp, $sp, -0x18
    addu $a3, $a0, $zero
    addu $t0, $a1, $zero
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    sll $v1, $a0, 2
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
    sll $v0, $a1, 1
    addu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sw $ra, 0x14($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v1
    lw $v1, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $s0, $v1, $v0
    andi $v1, $a2, 0xff
    addiu $v0, $zero, 0x14
    beq $v1, $v0, .Lcommands_8016a2ec
    addiu $v0, $zero, 0x1e
    beq $v1, $v0, .Lcommands_8016a30c
    nop
    j .Lcommands_8016a378
    sll $a0, $a3, 0x10
.Lcommands_8016a2ec:
    addiu $v0, $zero, 1
    sb $a2, 0x16($s0)
    jal audio_sequence_read_delta
    sb $v0, 0x27($s0)
    lw $v1, 4($s0)
    sw $v0, 0x88($s0)
    j .Lcommands_8016a3a0
    sw $v1, 0xc($s0)
.Lcommands_8016a30c:
    lbu $v1, 0x28($s0)
    nop
    andi $v0, $v1, 0xff
    bnez $v0, .Lcommands_8016a328
    sb $a2, 0x16($s0)
    j .Lcommands_8016a394
    sb $zero, 0x10($s0)
.Lcommands_8016a328:
    sltiu $v0, $v0, 0x7f
    beqz $v0, .Lcommands_8016a360
    addiu $v0, $v1, -1
    jal audio_sequence_read_delta
    sb $v0, 0x28($s0)
    lbu $v1, 0x28($s0)
    nop
    beqz $v1, .Lcommands_8016a358
    sw $v0, 0x88($s0)
    lw $v0, 0xc($s0)
    j .Lcommands_8016a3a0
    sw $v0, 4($s0)
.Lcommands_8016a358:
    j .Lcommands_8016a3a0
    sb $zero, 0x10($s0)
.Lcommands_8016a360:
    jal audio_sequence_read_delta
    nop
    lw $v0, 0xc($s0)
    sw $zero, 0x88($s0)
    j .Lcommands_8016a3a0
    sw $v0, 4($s0)
.Lcommands_8016a378:
    sra $a0, $a0, 0x10
    sll $a1, $t0, 0x10
    lbu $v0, 0x2a($s0)
    sra $a1, $a1, 0x10
    sb $a2, 0x16($s0)
    addiu $v0, $v0, 1
    sb $v0, 0x2a($s0)
.Lcommands_8016a394:
    jal audio_sequence_read_delta
    nop
    sw $v0, 0x88($s0)
.Lcommands_8016a3a0:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_loop_command

# exe/slus_004_22@0x8016A3B4..0x8016A42C
.globl audio_sequence_parameter_msb_command
.ent audio_sequence_parameter_msb_command
audio_sequence_parameter_msb_command:
    addiu $sp, $sp, -0x18
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    sll $v1, $a0, 2
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
    sll $v0, $a1, 1
    addu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sw $ra, 0x14($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v1
    lw $s0, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $s0, $s0, $v0
    lbu $v0, 0x29($s0)
    sb $a2, 0x13($s0)
    addiu $v0, $v0, 1
    jal audio_sequence_read_delta
    sb $v0, 0x29($s0)
    sw $v0, 0x88($s0)
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_parameter_msb_command

# exe/slus_004_22@0x8016A42C..0x8016A4A4
.globl audio_sequence_parameter_lsb_command
.ent audio_sequence_parameter_lsb_command
audio_sequence_parameter_lsb_command:
    addiu $sp, $sp, -0x18
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    sll $v1, $a0, 2
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
    sll $v0, $a1, 1
    addu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sw $ra, 0x14($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v1
    lw $s0, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $s0, $s0, $v0
    lbu $v0, 0x29($s0)
    sb $a2, 0x14($s0)
    addiu $v0, $v0, 1
    jal audio_sequence_read_delta
    sb $v0, 0x29($s0)
    sw $v0, 0x88($s0)
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_parameter_lsb_command

# exe/slus_004_22@0x8016A55C..0x8016A79C
.globl audio_sequence_controller_command
.ent audio_sequence_controller_command
audio_sequence_controller_command:
    addiu $sp, $sp, -0x18
    sll $v0, $a0, 0x10
    sra $t0, $v0, 0x10
    sll $v1, $t0, 2
    sll $v0, $a1, 0x10
    sra $a3, $v0, 0x10
    sll $v0, $a3, 1
    addu $v0, $v0, $a3
    sll $v0, $v0, 2
    subu $v0, $v0, $a3
    sll $v0, $v0, 2
    subu $v0, $v0, $a3
    sll $v0, $v0, 2
    sw $ra, 0x14($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v1
    lw $v1, %lo(audio_sequence_handles)($at)
    nop
    addu $s0, $v1, $v0
    lw $v1, 4($s0)
    andi $a2, $a2, 0xff
    addiu $v0, $v1, 1
    sw $v0, 4($s0)
    addiu $v0, $zero, 0x40
    lbu $v1, 0($v1)
    beq $a2, $v0, .Lcommands_8016a6c8
    slti $v0, $a2, 0x41
    beqz $v0, .Lcommands_8016a618
    addiu $v0, $zero, 7
    beq $a2, $v0, .Lcommands_8016a68c
    slti $v0, $a2, 8
    beqz $v0, .Lcommands_8016a5fc
    nop
    beqz $a2, .Lcommands_8016a668
    addiu $v0, $zero, 6
    beq $a2, $v0, .Lcommands_8016a678
    sll $a0, $a0, 0x10
    j .Lcommands_8016a770
    nop
.Lcommands_8016a5fc:
    addiu $v0, $zero, 0xa
    beq $a2, $v0, .Lcommands_8016a6a0
    addiu $v0, $zero, 0xb
    beq $a2, $v0, .Lcommands_8016a6b4
    sll $a0, $a0, 0x10
    j .Lcommands_8016a770
    nop
.Lcommands_8016a618:
    addiu $v0, $zero, 0x63
    beq $a2, $v0, .Lcommands_8016a704
    slti $v0, $a2, 0x64
    beqz $v0, .Lcommands_8016a644
    addiu $v0, $zero, 0x5b
    beq $a2, $v0, .Lcommands_8016a6dc
    addiu $v0, $zero, 0x62
    beq $a2, $v0, .Lcommands_8016a6f0
    sll $a0, $a0, 0x10
    j .Lcommands_8016a770
    nop
.Lcommands_8016a644:
    addiu $v0, $zero, 0x65
    beq $a2, $v0, .Lcommands_8016a72c
    slti $v0, $a2, 0x65
    bnez $v0, .Lcommands_8016a718
    addiu $v0, $zero, 0x79
    beq $a2, $v0, .Lcommands_8016a750
    sll $a0, $a0, 0x10
    j .Lcommands_8016a770
    nop
.Lcommands_8016a668:
    addu $a0, $t0, $zero
    addu $a1, $a3, $zero
    j .Lcommands_8016a77c
    sh $v1, 0x4c($s0)
.Lcommands_8016a678:
    addu $a0, $t0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 32)
    lw $v0, %lo(audio_sequence_voice_globals + 32)($v0)
    j .Lcommands_8016a73c
    addu $a1, $a3, $zero
.Lcommands_8016a68c:
    addu $a0, $t0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 36)
    lw $v0, %lo(audio_sequence_voice_globals + 36)($v0)
    j .Lcommands_8016a73c
    addu $a1, $a3, $zero
.Lcommands_8016a6a0:
    addu $a0, $t0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 40)
    lw $v0, %lo(audio_sequence_voice_globals + 40)($v0)
    j .Lcommands_8016a73c
    addu $a1, $a3, $zero
.Lcommands_8016a6b4:
    addu $a0, $t0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 44)
    lw $v0, %lo(audio_sequence_voice_globals + 44)($v0)
    j .Lcommands_8016a73c
    addu $a1, $a3, $zero
.Lcommands_8016a6c8:
    addu $a0, $t0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 44)
    lw $v0, %lo(audio_sequence_voice_globals + 44)($v0)
    j .Lcommands_8016a73c
    addu $a1, $a3, $zero
.Lcommands_8016a6dc:
    addu $a0, $t0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 64)
    lw $v0, %lo(audio_sequence_voice_globals + 64)($v0)
    j .Lcommands_8016a73c
    addu $a1, $a3, $zero
.Lcommands_8016a6f0:
    addu $a0, $t0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 48)
    lw $v0, %lo(audio_sequence_voice_globals + 48)($v0)
    j .Lcommands_8016a73c
    addu $a1, $a3, $zero
.Lcommands_8016a704:
    addu $a0, $t0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 52)
    lw $v0, %lo(audio_sequence_voice_globals + 52)($v0)
    j .Lcommands_8016a73c
    addu $a1, $a3, $zero
.Lcommands_8016a718:
    addu $a0, $t0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 56)
    lw $v0, %lo(audio_sequence_voice_globals + 56)($v0)
    j .Lcommands_8016a73c
    addu $a1, $a3, $zero
.Lcommands_8016a72c:
    addu $a0, $t0, $zero
    addu $a1, $a3, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 60)
    lw $v0, %lo(audio_sequence_voice_globals + 60)($v0)
.Lcommands_8016a73c:
    nop
    jalr $v0
    addu $a2, $v1, $zero
    j .Lcommands_8016a788
    nop
.Lcommands_8016a750:
    addu $a0, $t0, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 68)
    lw $v0, %lo(audio_sequence_voice_globals + 68)($v0)
    nop
    jalr $v0
    addu $a1, $a3, $zero
    j .Lcommands_8016a788
    nop
.Lcommands_8016a770:
    sra $a0, $a0, 0x10
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
.Lcommands_8016a77c:
    jal audio_sequence_read_delta
    nop
    sw $v0, 0x88($s0)
.Lcommands_8016a788:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_controller_command

# exe/slus_004_22@0x8016A79C..0x8016A974
.globl audio_sequence_tempo_command
.ent audio_sequence_tempo_command
audio_sequence_tempo_command:
    addiu $sp, $sp, -0x18
    lui $t0, 0x393
    sll $a2, $a0, 0x10
    sra $a2, $a2, 0xe
    sll $v1, $a1, 0x10
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sw $ra, 0x14($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a2
    lw $v1, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $s0, $v1, $v0
    lw $a3, 4($s0)
    ori $t0, $t0, 0x8700
    addiu $v0, $a3, 1
    sw $v0, 4($s0)
    lbu $v0, 0($a3)
    addiu $v1, $a3, 2
    sw $v1, 4($s0)
    lbu $a2, 1($a3)
    addiu $v1, $a3, 3
    sw $v1, 4($s0)
    lbu $v1, 2($a3)
    sll $v0, $v0, 0x10
    sll $a2, $a2, 8
    or $v0, $v0, $a2
    or $v0, $v0, $v1
    div $zero, $t0, $v0
    bnez $v0, .Lcommands_8016a834
    nop
    break 7
.Lcommands_8016a834:
    addiu $at, $zero, -1
    bne $v0, $at, .Lcommands_8016a84c
    lui $at, 0x8000
    bne $t0, $at, .Lcommands_8016a84c
    nop
    break 6
.Lcommands_8016a84c:
    mflo $t0
    lh $v0, 0x4a($s0)
    nop
    mult $v0, $t0
    lui $a2, %hi(audio_sequence_tick_rate)
    lw $a2, %lo(audio_sequence_tick_rate)($a2)
    nop
    sll $v1, $a2, 4
    sw $t0, 0x8c($s0)
    subu $t0, $v1, $a2
    sll $a3, $t0, 2
    mflo $t1
    sll $v0, $t1, 2
    addu $v0, $v0, $t1
    sll $v0, $v0, 1
    sltu $v0, $v0, $a3
    beqz $v0, .Lcommands_8016a8c0
    sll $v1, $a2, 2
    addu $v1, $v1, $a2
    sll $v0, $v1, 4
    subu $v0, $v0, $v1
    sll $v0, $v0, 3
    divu $zero, $v0, $t1
    bnez $t1, .Lcommands_8016a8b4
    nop
    break 7
.Lcommands_8016a8b4:
    mflo $v0
    j .Lcommands_8016a944
    sh $v0, 0x6e($s0)
.Lcommands_8016a8c0:
    lh $v1, 0x4a($s0)
    lw $v0, 0x8c($s0)
    nop
    mult $v1, $v0
    mflo $v1
    sll $v0, $v1, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 1
    divu $zero, $v0, $a3
    bnez $a3, .Lcommands_8016a8f0
    nop
    break 7
.Lcommands_8016a8f0:
    mflo $a2
    lh $v1, 0x4a($s0)
    lw $v0, 0x8c($s0)
    nop
    mult $v1, $v0
    mflo $v1
    sll $v0, $v1, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 1
    divu $zero, $v0, $a3
    bnez $a3, .Lcommands_8016a924
    nop
    break 7
.Lcommands_8016a924:
    mfhi $v1
    addiu $v0, $zero, -1
    sh $v0, 0x6e($s0)
    sll $v0, $t0, 1
    sltu $v0, $v0, $v1
    beqz $v0, .Lcommands_8016a948
    sh $a2, 0x70($s0)
    addiu $v0, $a2, 1
.Lcommands_8016a944:
    sh $v0, 0x70($s0)
.Lcommands_8016a948:
    sll $a0, $a0, 0x10
    sll $a1, $a1, 0x10
    sra $a0, $a0, 0x10
    jal audio_sequence_read_delta
    sra $a1, $a1, 0x10
    sw $v0, 0x88($s0)
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_tempo_command

# exe/slus_004_22@0x8016AA5C..0x8016AAD4
.globl audio_sequence_program_command
.ent audio_sequence_program_command
audio_sequence_program_command:
    addiu $sp, $sp, -0x18
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    sll $v1, $a0, 2
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
    sll $v0, $a1, 1
    addu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sw $ra, 0x14($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v1
    lw $s0, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $s0, $s0, $v0
    lbu $v0, 0x12($s0)
    nop
    addu $v0, $s0, $v0
    jal audio_sequence_read_delta
    sb $a2, 0x2c($v0)
    sw $v0, 0x88($s0)
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_program_command

# exe/slus_004_22@0x8016AAD4..0x8016AB8C
.globl audio_sequence_read_delta
.ent audio_sequence_read_delta
audio_sequence_read_delta:
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
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a0
    lw $v1, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $a1, $v1, $v0
    lw $v1, 4($a1)
    nop
    addiu $v0, $v1, 1
    sw $v0, 4($a1)
    lbu $a0, 0($v1)
    nop
    bnez $a0, .Lcommands_8016ab38
    andi $v0, $a0, 0x80
    j .Lcommands_8016ab84
    addu $v0, $zero, $zero
.Lcommands_8016ab38:
    beqz $v0, .Lcommands_8016ab70
    sll $v0, $a0, 2
    andi $a0, $a0, 0x7f
.Lcommands_8016ab44:
    lw $v1, 4($a1)
    nop
    addiu $v0, $v1, 1
    sw $v0, 4($a1)
    lbu $v1, 0($v1)
    sll $a0, $a0, 7
    andi $v0, $v1, 0x7f
    andi $v1, $v1, 0x80
    bnez $v1, .Lcommands_8016ab44
    addu $a0, $a0, $v0
    sll $v0, $a0, 2
.Lcommands_8016ab70:
    addu $v0, $v0, $a0
    lw $v1, 0x80($a1)
    sll $v0, $v0, 1
    addu $v1, $v1, $v0
    sw $v1, 0x80($a1)
.Lcommands_8016ab84:
    jr $ra
    nop
.end audio_sequence_read_delta

# exe/slus_004_22@0x80170680..0x80170690
.globl audio_sequence_clear_reverb_state
.ent audio_sequence_clear_reverb_state
audio_sequence_clear_reverb_state:
    lui $at, %hi(audio_sequence_voice_state_e1e0)
    sh $zero, %lo(audio_sequence_voice_state_e1e0)($at)
    jr $ra
    nop
.end audio_sequence_clear_reverb_state

# exe/slus_004_22@0x80170690..0x801706A4
.globl audio_sequence_set_reverb_state
.ent audio_sequence_set_reverb_state
audio_sequence_set_reverb_state:
    addiu $v0, $zero, 2
    lui $at, %hi(audio_sequence_voice_state_e1e0)
    sh $v0, %lo(audio_sequence_voice_state_e1e0)($at)
    jr $ra
    nop
.end audio_sequence_set_reverb_state
