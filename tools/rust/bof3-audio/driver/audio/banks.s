# SPDX-License-Identifier: MIT
# Owned VAB header opening and SPU allocation routines.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Header buffers and allocation records are mutable caller-owned RAM.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x80173C50..0x80173C80
.globl audio_bank_open_head_sticky
.ent audio_bank_open_head_sticky
audio_bank_open_head_sticky:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    addu $a3, $a2, $zero
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
    jal .Lbank_80173cb0
    addiu $a2, $zero, 1
    sll $v0, $v0, 0x10
    lw $ra, 0x10($sp)
    sra $v0, $v0, 0x10
    jr $ra
    addiu $sp, $sp, 0x18
.end audio_bank_open_head_sticky

# exe/slus_004_22@0x80173CB0..0x8017408C
.globl audio_bank_open_head
.ent audio_bank_open_head
audio_bank_open_head:
.Lbank_80173cb0:
    addiu $sp, $sp, -0x438
    sw $s3, 0x424($sp)
    addu $s3, $a0, $zero
    sw $s0, 0x418($sp)
    addu $s0, $a1, $zero
    sw $s1, 0x41c($sp)
    addiu $s1, $zero, 0x10
    sw $s5, 0x42c($sp)
    addu $s5, $a3, $zero
    sw $s4, 0x428($sp)
    addu $s4, $a2, $zero
    sw $ra, 0x430($sp)
    jal .Lbank_80171014
    sw $s2, 0x420($sp)
    addiu $v1, $zero, 1
    bne $v0, $v1, .Lbank_80173cfc
    addu $s2, $s0, $zero
    j .Lbank_80174064
    addiu $v0, $zero, -1
.Lbank_80173cfc:
    jal audio_spu_set_transfer_mode
    addiu $a0, $zero, 1
    sll $v0, $s0, 0x10
    sra $v1, $v0, 0x10
    slti $v0, $v1, 0x10
    beqz $v0, .Lbank_80173dc0
    addiu $v0, $zero, -1
    bne $v1, $v0, .Lbank_80173d6c
    addu $a1, $zero, $zero
.Lbank_80173d20:
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_sequence_voice_flags)($at)
    nop
    beqz $v0, .Lbank_80173d50
    addiu $v0, $zero, 1
    addiu $a1, $a1, 1
    slti $v0, $a1, 0x10
    bnez $v0, .Lbank_80173d20
    sll $v0, $s1, 0x10
    j .Lbank_80173db0
    nop
.Lbank_80173d50:
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $a1
    sb $v0, %lo(audio_sequence_voice_flags)($at)
    lui $v0, %hi(audio_sequence_voice_state_0bd0)
    lhu $v0, %lo(audio_sequence_voice_state_0bd0)($v0)
    j .Lbank_80173da0
    addu $s1, $a1, $zero
.Lbank_80173d6c:
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $v1
    lbu $v0, %lo(audio_sequence_voice_flags)($at)
    nop
    bnez $v0, .Lbank_80173db0
    sll $v0, $s1, 0x10
    addiu $v0, $zero, 1
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $v1
    sb $v0, %lo(audio_sequence_voice_flags)($at)
    lui $v0, %hi(audio_sequence_voice_state_0bd0)
    lhu $v0, %lo(audio_sequence_voice_state_0bd0)($v0)
    addu $s1, $s2, $zero
.Lbank_80173da0:
    addiu $v0, $v0, 1
    lui $at, %hi(audio_sequence_voice_state_0bd0)
    sh $v0, %lo(audio_sequence_voice_state_0bd0)($at)
    sll $v0, $s1, 0x10
.Lbank_80173db0:
    sra $a3, $v0, 0x10
    slti $v0, $a3, 0x10
    bnez $v0, .Lbank_80173dd0
    addu $a2, $s3, $zero
.Lbank_80173dc0:
    jal audio_spu_set_transfer_mode
    addu $a0, $zero, $zero
    j .Lbank_80174064
    addiu $v0, $zero, -1
.Lbank_80173dd0:
    sll $v0, $a3, 2
    lui $at, %hi(audio_sequence_voice_globals + 0x650)
    addu $at, $at, $v0
    sw $a2, %lo(audio_sequence_voice_globals + 0x650)($at)
    lui $v1, 0x56
    addu $a0, $s3, $zero
    lw $a1, 0($a0)
    ori $v1, $v1, 0x4142
    srl $v0, $a1, 8
    beq $v0, $v1, .Lbank_80173e10
    addiu $a2, $a2, 0x20
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $a3
    sb $zero, %lo(audio_sequence_voice_flags)($at)
    j .Lbank_80173f84
    addu $a0, $zero, $zero
.Lbank_80173e10:
    andi $v1, $a1, 0xff
    addiu $v0, $zero, 0x70
    bne $v1, $v0, .Lbank_80173e38
    addiu $v0, $zero, 0x40
    lw $v0, 4($a0)
    nop
    slti $v0, $v0, 5
    bnez $v0, .Lbank_80173e38
    addiu $v0, $zero, 0x40
    addiu $v0, $zero, 0x80
.Lbank_80173e38:
    lui $at, %hi(audio_sequence_voice_globals + 0x6fc)
    sh $v0, %lo(audio_sequence_voice_globals + 0x6fc)($at)
    lhu $v0, 0x12($a0)
    lui $v1, %hi(audio_sequence_voice_globals + 0x6fc)
    lh $v1, %lo(audio_sequence_voice_globals + 0x6fc)($v1)
    nop
    slt $v0, $v1, $v0
    bnez $v0, .Lbank_80173f6c
    addu $s2, $a2, $zero
    sll $v0, $s1, 0x10
    sra $v0, $v0, 0xe
    lui $at, %hi(audio_sequence_voice_globals + 0x5b0)
    addu $at, $at, $v0
    sw $a2, %lo(audio_sequence_voice_globals + 0x5b0)($at)
    sll $v0, $v1, 4
    addu $a2, $s2, $v0
    addu $a1, $zero, $zero
    blez $v1, .Lbank_80173eb0
    addu $s0, $zero, $zero
    addu $a3, $v1, $zero
    addu $v1, $s2, $zero
.Lbank_80173e8c:
    lbu $v0, 0($v1)
    nop
    beqz $v0, .Lbank_80173ea0
    sw $s0, 8($v1)
    addiu $s0, $s0, 1
.Lbank_80173ea0:
    addiu $a1, $a1, 1
    slt $v0, $a1, $a3
    bnez $v0, .Lbank_80173e8c
    addiu $v1, $v1, 0x10
.Lbank_80173eb0:
    addu $s0, $zero, $zero
    addiu $a1, $sp, 0x10
    sll $v0, $s1, 0x10
    sra $v0, $v0, 0xe
    addiu $t0, $sp, 0x410
    lui $at, %hi(audio_sequence_voice_globals + 0x698)
    addu $at, $at, $v0
    sw $a2, %lo(audio_sequence_voice_globals + 0x698)($at)
    lhu $v0, 0x12($a0)
    lbu $s3, 0x16($a0)
    sll $v0, $v0, 9
    addu $a2, $a2, $v0
    sll $v0, $s3, 2
    addu $a3, $v0, $a1
.Lbank_80173ee8:
    slt $v0, $a3, $a1
    bnez $v0, .Lbank_80173f1c
    nop
    lw $v0, 4($a0)
    lhu $v1, 0($a2)
    slti $v0, $v0, 5
    bnez $v0, .Lbank_80173f0c
    sll $v0, $v1, 2
    sll $v0, $v1, 3
.Lbank_80173f0c:
    sw $v0, 0($a1)
    lw $v0, 0($a1)
    nop
    addu $s0, $s0, $v0
.Lbank_80173f1c:
    addiu $a1, $a1, 4
    slt $v0, $a1, $t0
    bnez $v0, .Lbank_80173ee8
    addiu $a2, $a2, 2
    sll $v0, $s4, 0x10
    bnez $v0, .Lbank_80173f58
    addu $a3, $s5, $zero
    jal .Lbank_8017408c
    addu $a0, $s0, $zero
    addu $a3, $v0, $zero
    addiu $v0, $zero, -1
    bne $a3, $v0, .Lbank_80173f5c
    addu $v1, $a3, $s0
    j .Lbank_80173f70
    addu $a0, $zero, $zero
.Lbank_80173f58:
    addu $v1, $a3, $s0
.Lbank_80173f5c:
    lui $v0, 8
    sltu $v0, $v0, $v1
    beqz $v0, .Lbank_80173fac
    addu $s0, $zero, $zero
.Lbank_80173f6c:
    addu $a0, $zero, $zero
.Lbank_80173f70:
    sll $v0, $s1, 0x10
    sra $v0, $v0, 0x10
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $v0
    sb $zero, %lo(audio_sequence_voice_flags)($at)
.Lbank_80173f84:
    jal audio_spu_set_transfer_mode
    nop
    lui $v1, %hi(audio_sequence_voice_state_0bd0)
    lhu $v1, %lo(audio_sequence_voice_state_0bd0)($v1)
    nop
    addiu $v1, $v1, -1
    lui $at, %hi(audio_sequence_voice_state_0bd0)
    sh $v1, %lo(audio_sequence_voice_state_0bd0)($at)
    j .Lbank_80174064
    addiu $v0, $zero, -1
.Lbank_80173fac:
    sll $v0, $s1, 0x10
    sra $v0, $v0, 0xe
    lui $at, %hi(audio_bank_spu_bases)
    addu $at, $at, $v0
    sw $a3, %lo(audio_bank_spu_bases)($at)
    andi $v1, $s3, 0xff
    slti $v0, $v1, 0
    bnez $v0, .Lbank_8017403c
    addu $a1, $zero, $zero
    addu $a2, $v1, $zero
    addiu $a0, $sp, 0x10
.Lbank_80173fd8:
    lw $v0, 0($a0)
    nop
    addu $s0, $s0, $v0
    andi $v0, $a1, 1
    bnez $v0, .Lbank_80174010
    srl $v1, $a1, 0x1f
    addu $v1, $a1, $v1
    sra $v1, $v1, 1
    sll $v1, $v1, 4
    addu $v1, $v1, $s2
    addu $v0, $a3, $s0
    srl $v0, $v0, 3
    j .Lbank_8017402c
    sh $v0, 0xc($v1)
.Lbank_80174010:
    addu $v1, $a1, $v1
    sra $v1, $v1, 1
    sll $v1, $v1, 4
    addu $v1, $v1, $s2
    addu $v0, $a3, $s0
    srl $v0, $v0, 3
    sh $v0, 0xe($v1)
.Lbank_8017402c:
    addiu $a1, $a1, 1
    slt $v0, $a2, $a1
    beqz $v0, .Lbank_80173fd8
    addiu $a0, $a0, 4
.Lbank_8017403c:
    sll $v0, $s1, 0x10
    sra $v0, $v0, 0x10
    sll $v1, $v0, 2
    lui $at, %hi(audio_bank_body_sizes)
    addu $at, $at, $v1
    sw $s0, %lo(audio_bank_body_sizes)($at)
    addiu $v1, $zero, 2
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $v0
    sb $v1, %lo(audio_sequence_voice_flags)($at)
.Lbank_80174064:
    lw $ra, 0x430($sp)
    lw $s5, 0x42c($sp)
    lw $s4, 0x428($sp)
    lw $s3, 0x424($sp)
    lw $s2, 0x420($sp)
    lw $s1, 0x41c($sp)
    lw $s0, 0x418($sp)
    addiu $sp, $sp, 0x438
    jr $ra
    nop
.end audio_bank_open_head

# exe/slus_004_22@0x8017408C..0x80174354
.globl audio_spu_allocate
.ent audio_spu_allocate
audio_spu_allocate:
.Lbank_8017408c:
    lui $v0, %hi(audio_spu_state_3b24)
    lw $v0, %lo(audio_spu_state_3b24)($v0)
    addiu $sp, $sp, -0x28
    sw $s1, 0x14($sp)
    addu $s1, $a0, $zero
    sw $s0, 0x10($sp)
    addu $s0, $zero, $zero
    sw $s2, 0x18($sp)
    addiu $s2, $zero, -1
    sw $ra, 0x20($sp)
    bnez $v0, .Lbank_801740c4
    sw $s3, 0x1c($sp)
    j .Lbank_801740e0
    addu $s3, $zero, $zero
.Lbank_801740c4:
    lui $v0, 1
    lui $v1, %hi(audio_spu_state_3b28)
    lw $v1, %lo(audio_spu_state_3b28)($v1)
    lui $a0, %hi(audio_spu_transfer_shift)
    lw $a0, %lo(audio_spu_transfer_shift)($a0)
    subu $v0, $v0, $v1
    sllv $s3, $v0, $a0
.Lbank_801740e0:
    lui $v1, %hi(audio_spu_state_3ae0)
    lw $v1, %lo(audio_spu_state_3ae0)($v1)
    nop
    nor $v0, $zero, $v1
    and $v0, $s1, $v0
    beqz $v0, .Lbank_80174100
    addu $a0, $s1, $zero
    addu $a0, $s1, $v1
.Lbank_80174100:
    lui $v1, %hi(audio_spu_transfer_shift)
    lw $v1, %lo(audio_spu_transfer_shift)($v1)
    lui $v0, %hi(audio_spu_state_3f88)
    lw $v0, %lo(audio_spu_state_3f88)($v0)
    srav $s1, $a0, $v1
    sllv $s1, $s1, $v1
    lw $v0, 0($v0)
    lui $v1, 0x4000
    and $v0, $v0, $v1
    beqz $v0, .Lbank_80174134
    nop
    j .Lbank_801741b8
    addu $s2, $zero, $zero
.Lbank_80174134:
    jal .Lbank_80173918
    nop
    lui $v1, %hi(audio_spu_state_3f80)
    lw $v1, %lo(audio_spu_state_3f80)($v1)
    nop
    slt $v0, $s0, $v1
    beqz $v0, .Lbank_801741b8
    sll $v0, $s0, 3
    lui $a3, 0x4000
    lui $a2, 0x8000
    addu $a1, $v1, $zero
    lui $v1, %hi(audio_spu_state_3f88)
    lw $v1, %lo(audio_spu_state_3f88)($v1)
    nop
    addu $a0, $v0, $v1
.Lbank_80174170:
    lw $v1, 0($a0)
    nop
    and $v0, $v1, $a3
    bnez $v0, .Lbank_801741a0
    and $v0, $v1, $a2
    beqz $v0, .Lbank_801741a8
    nop
    lw $v0, 4($a0)
    nop
    sltu $v0, $v0, $s1
    bnez $v0, .Lbank_801741a8
    nop
.Lbank_801741a0:
    j .Lbank_801741b8
    addu $s2, $s0, $zero
.Lbank_801741a8:
    addiu $s0, $s0, 1
    slt $v0, $s0, $a1
    bnez $v0, .Lbank_80174170
    addiu $a0, $a0, 8
.Lbank_801741b8:
    addiu $v0, $zero, -1
    beq $s2, $v0, .Lbank_80174334
    sll $s0, $s2, 3
    lui $a3, %hi(audio_spu_state_3f88)
    lw $a3, %lo(audio_spu_state_3f88)($a3)
    nop
    addu $a2, $s0, $a3
    lw $v1, 0($a2)
    lui $t0, 0x4000
    and $v0, $v1, $t0
    beqz $v0, .Lbank_80174288
    nop
    lui $v0, %hi(audio_spu_state_3f80)
    lw $v0, %lo(audio_spu_state_3f80)($v0)
    nop
    slt $v0, $s2, $v0
    beqz $v0, .Lbank_80174334
    addiu $v0, $zero, -1
    lw $v0, 4($a2)
    nop
    subu $v0, $v0, $s3
    sltu $v0, $v0, $s1
    bnez $v0, .Lbank_80174334
    addiu $v0, $zero, -1
    lui $a1, 0xfff
    ori $a1, $a1, 0xffff
    addiu $a0, $s2, 1
    sll $v1, $a0, 3
    lw $v0, 0($a2)
    addu $v1, $v1, $a3
    and $v0, $v0, $a1
    addu $v0, $v0, $s1
    or $v0, $v0, $t0
    sw $v0, 0($v1)
    lw $v0, 4($a2)
    nop
    subu $v0, $v0, $s1
    sw $v0, 4($v1)
    lw $v0, 0($a2)
    lui $at, %hi(audio_spu_state_3f84)
    sw $a0, %lo(audio_spu_state_3f84)($at)
    sw $s1, 4($a2)
    and $v0, $v0, $a1
    jal .Lbank_80173918
    sw $v0, 0($a2)
    lui $v0, %hi(audio_spu_state_3f88)
    lw $v0, %lo(audio_spu_state_3f88)($v0)
    nop
    addu $v0, $s0, $v0
    lw $v0, 0($v0)
    j .Lbank_80174334
    nop
.Lbank_80174288:
    lw $t0, 4($a2)
    nop
    sltu $v0, $s1, $t0
    beqz $v0, .Lbank_801742f8
    lui $a0, 0xfff
    lui $a2, %hi(audio_spu_state_3f84)
    lw $a2, %lo(audio_spu_state_3f84)($a2)
    lui $v0, %hi(audio_spu_state_3f80)
    lw $v0, %lo(audio_spu_state_3f80)($v0)
    nop
    slt $v0, $a2, $v0
    beqz $v0, .Lbank_801742f8
    addu $v1, $v1, $s1
    sll $v0, $a2, 3
    addu $v0, $v0, $a3
    lui $a0, 0x8000
    or $v1, $v1, $a0
    lw $a1, 0($v0)
    lw $a0, 4($v0)
    sw $v1, 0($v0)
    subu $v1, $t0, $s1
    sw $v1, 4($v0)
    addiu $v1, $a2, 1
    lui $at, %hi(audio_spu_state_3f84)
    sw $v1, %lo(audio_spu_state_3f84)($at)
    sw $a1, 8($v0)
    sw $a0, 0xc($v0)
    lui $a0, 0xfff
.Lbank_801742f8:
    lui $v1, %hi(audio_spu_state_3f88)
    lw $v1, %lo(audio_spu_state_3f88)($v1)
    sll $s0, $s2, 3
    addu $v1, $s0, $v1
    lw $v0, 0($v1)
    ori $a0, $a0, 0xffff
    sw $s1, 4($v1)
    and $v0, $v0, $a0
    jal .Lbank_80173918
    sw $v0, 0($v1)
    lui $v0, %hi(audio_spu_state_3f88)
    lw $v0, %lo(audio_spu_state_3f88)($v0)
    nop
    addu $s0, $s0, $v0
    lw $v0, 0($s0)
.Lbank_80174334:
    lw $ra, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x28
    jr $ra
    nop
.end audio_spu_allocate

# exe/slus_004_22@0x80173918..0x80173C20
.globl audio_spu_coalesce_allocations
.ent audio_spu_coalesce_allocations
audio_spu_coalesce_allocations:
.Lbank_80173918:
    lui $v0, %hi(audio_spu_state_3f84)
    lw $v0, %lo(audio_spu_state_3f84)($v0)
    nop
    bltz $v0, .Lbank_801739f8
    addu $t1, $zero, $zero
    lui $t4, 0x8000
    lui $t2, 0x2fff
    ori $t2, $t2, 0xffff
    lui $t3, 0xfff
    lui $t0, %hi(audio_spu_state_3f88)
    lw $t0, %lo(audio_spu_state_3f88)($t0)
    ori $t3, $t3, 0xffff
    addu $a3, $t0, $zero
.Lbank_8017394c:
    lw $v0, 0($a3)
    nop
    and $v0, $v0, $t4
    beqz $v0, .Lbank_801739d0
    addiu $a2, $t1, 1
    sll $v0, $a2, 3
    addu $v1, $v0, $t0
.Lbank_80173968:
    lw $v0, 0($v1)
    nop
    bne $v0, $t2, .Lbank_80173980
    addiu $v1, $v1, 8
    j .Lbank_80173968
    addiu $a2, $a2, 1
.Lbank_80173980:
    sll $v0, $a2, 3
    addu $a1, $v0, $t0
    lw $v1, 0($a1)
    nop
    and $v0, $v1, $t4
    beqz $v0, .Lbank_801739d0
    and $v0, $v1, $t3
    lw $v1, 0($a3)
    lw $a0, 4($a3)
    and $v1, $v1, $t3
    addu $v1, $v1, $a0
    bne $v0, $v1, .Lbank_801739d0
    nop
    sw $t2, 0($a1)
    lw $v0, 4($a3)
    lw $v1, 4($a1)
    nop
    addu $v0, $v0, $v1
    j .Lbank_801739d8
    sw $v0, 4($a3)
.Lbank_801739d0:
    addiu $a3, $a3, 8
    addiu $t1, $t1, 1
.Lbank_801739d8:
    lui $v0, %hi(audio_spu_state_3f84)
    lw $v0, %lo(audio_spu_state_3f84)($v0)
    nop
    slt $v0, $v0, $t1
    beqz $v0, .Lbank_8017394c
    nop
    lui $v0, %hi(audio_spu_state_3f84)
    lw $v0, %lo(audio_spu_state_3f84)($v0)
.Lbank_801739f8:
    nop
    bltz $v0, .Lbank_80173a40
    addu $t1, $zero, $zero
    lui $a1, 0x2fff
    ori $a1, $a1, 0xffff
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_spu_state_3f88)
    lw $v1, %lo(audio_spu_state_3f88)($v1)
.Lbank_80173a18:
    nop
    lw $v0, 4($v1)
    nop
    bnez $v0, .Lbank_80173a30
    nop
    sw $a1, 0($v1)
.Lbank_80173a30:
    addiu $t1, $t1, 1
    slt $v0, $a0, $t1
    beqz $v0, .Lbank_80173a18
    addiu $v1, $v1, 8
.Lbank_80173a40:
    lui $v1, %hi(audio_spu_state_3f84)
    lw $v1, %lo(audio_spu_state_3f84)($v1)
    nop
    bltz $v1, .Lbank_80173b0c
    addu $t1, $zero, $zero
    lui $t6, 0x4000
    lui $t4, 0xfff
    lui $t5, %hi(audio_spu_state_3f88)
    lw $t5, %lo(audio_spu_state_3f88)($t5)
    ori $t4, $t4, 0xffff
    addu $t2, $t5, $zero
.Lbank_80173a6c:
    lw $v0, 0($t2)
    nop
    and $v0, $v0, $t6
    bnez $v0, .Lbank_80173b0c
    nop
    addiu $a2, $t1, 1
    slt $v0, $v1, $a2
    bnez $v0, .Lbank_80173af4
    sll $v0, $a2, 3
    addu $t0, $t2, $zero
    lui $t3, %hi(audio_spu_state_3f84)
    lw $t3, %lo(audio_spu_state_3f84)($t3)
    addu $a0, $v0, $t5
.Lbank_80173aa0:
    lw $a1, 0($a0)
    nop
    and $v0, $a1, $t6
    bnez $v0, .Lbank_80173af4
    and $v0, $a1, $t4
    lw $a3, 0($t0)
    nop
    and $v1, $a3, $t4
    sltu $v0, $v0, $v1
    beqz $v0, .Lbank_80173ae4
    nop
    sw $a1, 0($t0)
    lw $v0, 4($a0)
    lw $v1, 4($t0)
    sw $v0, 4($t0)
    sw $a3, 0($a0)
    sw $v1, 4($a0)
.Lbank_80173ae4:
    addiu $a2, $a2, 1
    slt $v0, $t3, $a2
    beqz $v0, .Lbank_80173aa0
    addiu $a0, $a0, 8
.Lbank_80173af4:
    lui $v1, %hi(audio_spu_state_3f84)
    lw $v1, %lo(audio_spu_state_3f84)($v1)
    addiu $t1, $t1, 1
    slt $v0, $v1, $t1
    beqz $v0, .Lbank_80173a6c
    addiu $t2, $t2, 8
.Lbank_80173b0c:
    lui $a1, %hi(audio_spu_state_3f84)
    lw $a1, %lo(audio_spu_state_3f84)($a1)
    nop
    bltz $a1, .Lbank_80173b90
    addu $t1, $zero, $zero
    lui $t0, 0x4000
    lui $a3, 0x2fff
    lui $a2, %hi(audio_spu_state_3f88)
    lw $a2, %lo(audio_spu_state_3f88)($a2)
    ori $a3, $a3, 0xffff
    addu $a0, $a2, $zero
.Lbank_80173b38:
    lw $v1, 0($a0)
    nop
    and $v0, $v1, $t0
    bnez $v0, .Lbank_80173b90
    nop
    bne $v1, $a3, .Lbank_80173b78
    sll $v0, $a1, 3
    addu $v0, $v0, $a2
    lw $v1, 0($v0)
    nop
    sw $v1, 0($a0)
    lw $v0, 4($v0)
    lui $at, %hi(audio_spu_state_3f84)
    sw $t1, %lo(audio_spu_state_3f84)($at)
    j .Lbank_80173b90
    sw $v0, 4($a0)
.Lbank_80173b78:
    lui $a1, %hi(audio_spu_state_3f84)
    lw $a1, %lo(audio_spu_state_3f84)($a1)
    addiu $t1, $t1, 1
    slt $v0, $a1, $t1
    beqz $v0, .Lbank_80173b38
    addiu $a0, $a0, 8
.Lbank_80173b90:
    lui $v0, %hi(audio_spu_state_3f84)
    lw $v0, %lo(audio_spu_state_3f84)($v0)
    nop
    addiu $t1, $v0, -1
    bltz $t1, .Lbank_80173c18
    sll $v0, $t1, 3
    lui $t0, 0x8000
    lui $a2, 0xfff
    ori $a2, $a2, 0xffff
    lui $a3, 0x4000
    lui $a1, %hi(audio_spu_state_3f88)
    lw $a1, %lo(audio_spu_state_3f88)($a1)
    nop
    addu $a0, $v0, $a1
.Lbank_80173bc8:
    lw $v1, 0($a0)
    nop
    and $v0, $v1, $t0
    beqz $v0, .Lbank_80173c18
    and $v0, $v1, $a2
    lui $v1, %hi(audio_spu_state_3f84)
    lw $v1, %lo(audio_spu_state_3f84)($v1)
    or $v0, $v0, $a3
    sw $v0, 0($a0)
    lw $v0, 4($a0)
    lui $at, %hi(audio_spu_state_3f84)
    sw $t1, %lo(audio_spu_state_3f84)($at)
    sll $v1, $v1, 3
    addu $v1, $v1, $a1
    lw $v1, 4($v1)
    addiu $t1, $t1, -1
    addu $v0, $v0, $v1
    sw $v0, 4($a0)
    bgez $t1, .Lbank_80173bc8
    addiu $a0, $a0, -8
.Lbank_80173c18:
    jr $ra
    nop
.end audio_spu_coalesce_allocations

# exe/slus_004_22@0x80171014..0x8017102C
.globl audio_spu_transfer_busy
.ent audio_spu_transfer_busy
audio_spu_transfer_busy:
.Lbank_80171014:
    lui $v0, %hi(audio_spu_state_3ae4)
    lw $v0, %lo(audio_spu_state_3ae4)($v0)
    nop
    xori $v0, $v0, 1
    jr $ra
    sltu $v0, $zero, $v0
.end audio_spu_transfer_busy

.section .bss
.balign 4
# exe/slus_004_22@0x80190B90, 64 bytes.
.globl audio_bank_body_sizes
audio_bank_body_sizes:
    .space 64
# exe/slus_004_22@0x80190BD0, open count and six-byte gap.
.globl audio_sequence_voice_state_0bd0
audio_sequence_voice_state_0bd0:
    .space 2
    .space 6
# exe/slus_004_22@0x80190BD8, 64 bytes.
.globl audio_bank_spu_bases
audio_bank_spu_bases:
    .space 64

# exe/slus_004_22@0x80190C18, history and release mask.
.globl audio_sequence_envelope_history
audio_sequence_envelope_history:
    .space 64
.globl audio_sequence_release_mask
audio_sequence_release_mask:
    .space 4
