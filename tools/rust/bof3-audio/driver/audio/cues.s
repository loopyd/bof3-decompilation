# SPDX-License-Identifier: MIT
# Owned cue dispatcher, seven handlers and cached voice-volume update.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Lookup aliases, callbacks and working state remain live shared storage.
.set noreorder
.set noat

.section .text
.balign 4
# exe/slus_004_22@0x8015DF18..0x8015E994
.globl audio_cue_dispatch
.ent audio_cue_dispatch
audio_cue_dispatch:
    addiu $sp, $sp, -0x28
    andi $v0, $a0, 0xf00
    srl $v0, $v0, 8
    andi $v1, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x100)
    sh $v0, %lo(audio_cue_work + 0x100)($at)
    sll $v0, $v0, 2
    sw $ra, 0x24($sp)
    sw $s0, 0x20($sp)
    lui $at, %hi(audio_cue_dispatch_prefix + 0x4)
    addu $at, $at, $v0
    lw $v0, %lo(audio_cue_dispatch_prefix + 0x4)($at)
    lui $at, %hi(audio_cue_work + 0xfc)
    sh $v1, %lo(audio_cue_work + 0xfc)($at)
    jalr $v0
    addu $s0, $a0, $zero
    lui $a1, %hi(audio_cue_work + 0xec)
    lh $a1, %lo(audio_cue_work + 0xec)($a1)
    addiu $v0, $zero, 0x80
    bne $a1, $v0, .Leffects_8015e154
    addiu $v0, $zero, 1
    lui $v1, %hi(audio_cue_work + 0xdc)
    lh $v1, %lo(audio_cue_work + 0xdc)($v1)
    nop
    beq $v1, $v0, .Leffects_8015e070
    slti $v0, $v1, 2
    beqz $v0, .Leffects_8015df98
    nop
    beqz $v1, .Leffects_8015e0d8
    addiu $v0, $zero, 0x80
    j .Leffects_8015e13c
    andi $v0, $s0, 0xf000
.Leffects_8015df98:
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_8015e00c
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_8015e13c
    andi $v0, $s0, 0xf000
    lui $a0, %hi(audio_cue_work + 0x9c)
    lh $a0, %lo(audio_cue_work + 0x9c)($a0)
    lui $v1, %hi(audio_cue_work + 0x94)
    lh $v1, %lo(audio_cue_work + 0x94)($v1)
    subu $v0, $a1, $a0
    mult $v1, $v0
    mflo $v1
    bgez $v1, .Leffects_8015dfd4
    nop
    addiu $v1, $v1, 0x7f
.Leffects_8015dfd4:
    lui $v0, %hi(audio_cue_work + 0x98)
    lh $v0, %lo(audio_cue_work + 0x98)($v0)
    nop
    mult $v0, $a0
    sra $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x94)
    sh $v0, %lo(audio_cue_work + 0x94)($at)
    mflo $a0
    bgez $a0, .Leffects_8015e004
    sra $v0, $a0, 7
    addiu $a0, $a0, 0x7f
    sra $v0, $a0, 7
.Leffects_8015e004:
    lui $at, %hi(audio_cue_work + 0x98)
    sh $v0, %lo(audio_cue_work + 0x98)($at)
.Leffects_8015e00c:
    addiu $v0, $zero, 0x80
    lui $a0, %hi(audio_cue_work + 0x74)
    lh $a0, %lo(audio_cue_work + 0x74)($a0)
    lui $v1, %hi(audio_cue_work + 0x6c)
    lh $v1, %lo(audio_cue_work + 0x6c)($v1)
    subu $v0, $v0, $a0
    mult $v1, $v0
    mflo $v1
    bgez $v1, .Leffects_8015e038
    nop
    addiu $v1, $v1, 0x7f
.Leffects_8015e038:
    lui $v0, %hi(audio_cue_work + 0x70)
    lh $v0, %lo(audio_cue_work + 0x70)($v0)
    nop
    mult $v0, $a0
    sra $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x6c)
    sh $v0, %lo(audio_cue_work + 0x6c)($at)
    mflo $a0
    bgez $a0, .Leffects_8015e068
    sra $v0, $a0, 7
    addiu $a0, $a0, 0x7f
    sra $v0, $a0, 7
.Leffects_8015e068:
    lui $at, %hi(audio_cue_work + 0x70)
    sh $v0, %lo(audio_cue_work + 0x70)($at)
.Leffects_8015e070:
    addiu $v0, $zero, 0x80
    lui $a0, %hi(audio_cue_work + 0x4c)
    lh $a0, %lo(audio_cue_work + 0x4c)($a0)
    lui $v1, %hi(audio_cue_work + 0x44)
    lh $v1, %lo(audio_cue_work + 0x44)($v1)
    subu $v0, $v0, $a0
    mult $v1, $v0
    mflo $v1
    bgez $v1, .Leffects_8015e09c
    nop
    addiu $v1, $v1, 0x7f
.Leffects_8015e09c:
    lui $v0, %hi(audio_cue_work + 0x48)
    lh $v0, %lo(audio_cue_work + 0x48)($v0)
    nop
    mult $v0, $a0
    sra $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x44)
    sh $v0, %lo(audio_cue_work + 0x44)($at)
    mflo $a0
    bgez $a0, .Leffects_8015e0cc
    sra $v0, $a0, 7
    addiu $a0, $a0, 0x7f
    sra $v0, $a0, 7
.Leffects_8015e0cc:
    lui $at, %hi(audio_cue_work + 0x48)
    sh $v0, %lo(audio_cue_work + 0x48)($at)
    addiu $v0, $zero, 0x80
.Leffects_8015e0d8:
    lui $a0, %hi(audio_cue_work + 0x24)
    lh $a0, %lo(audio_cue_work + 0x24)($a0)
    lui $v1, %hi(audio_cue_work + 0x1c)
    lh $v1, %lo(audio_cue_work + 0x1c)($v1)
    subu $v0, $v0, $a0
    mult $v1, $v0
    mflo $v1
    bgez $v1, .Leffects_8015e100
    nop
    addiu $v1, $v1, 0x7f
.Leffects_8015e100:
    lui $v0, %hi(audio_cue_work + 0x20)
    lh $v0, %lo(audio_cue_work + 0x20)($v0)
    nop
    mult $v0, $a0
    sra $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v0, %lo(audio_cue_work + 0x1c)($at)
    mflo $a0
    bgez $a0, .Leffects_8015e130
    sra $v0, $a0, 7
    addiu $a0, $a0, 0x7f
    sra $v0, $a0, 7
.Leffects_8015e130:
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v0, %lo(audio_cue_work + 0x20)($at)
    andi $v0, $s0, 0xf000
.Leffects_8015e13c:
    bnez $v0, .Leffects_8015e154
    addiu $v0, $zero, 0x17ff
    lui $at, %hi(audio_cue_work + 0xd0)
    sh $v0, %lo(audio_cue_work + 0xd0)($at)
    lui $at, %hi(audio_cue_work + 0xd4)
    sh $v0, %lo(audio_cue_work + 0xd4)($at)
.Leffects_8015e154:
    lui $v0, %hi(audio_cue_dispatch_prefix + 0x2)
    lb $v0, %lo(audio_cue_dispatch_prefix + 0x2)($v0)
    nop
    bnez $v0, .Leffects_8015e40c
    addiu $v0, $zero, 1
    lui $v1, %hi(audio_cue_work + 0xdc)
    lh $v1, %lo(audio_cue_work + 0xdc)($v1)
    nop
    beq $v1, $v0, .Leffects_8015e2c0
    slti $v0, $v1, 2
    beqz $v0, .Leffects_8015e194
    nop
    beqz $v1, .Leffects_8015e34c
    nop
    j .Leffects_8015e980
    nop
.Leffects_8015e194:
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_8015e234
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_8015e980
    nop
    lui $v0, %hi(audio_cue_work + 0x84)
    lh $v0, %lo(audio_cue_work + 0x84)($v0)
    lui $a0, %hi(audio_cue_work + 0x88)
    lh $a0, %lo(audio_cue_work + 0x88)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work + 0x78)
    lh $a2, %lo(audio_cue_work + 0x78)($a2)
    lui $a3, %hi(audio_cue_work + 0x80)
    lh $a3, %lo(audio_cue_work + 0x80)($a3)
    lui $v1, %hi(audio_cue_work + 0x90)
    lh $v1, %lo(audio_cue_work + 0x90)($v1)
    lui $t0, %hi(audio_cue_work + 0x94)
    lh $t0, %lo(audio_cue_work + 0x94)($t0)
    lui $t1, %hi(audio_cue_work + 0x98)
    lh $t1, %lo(audio_cue_work + 0x98)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x8c)
    sh $a0, %lo(audio_cue_work + 0x8c)($at)
    bne $v1, $v0, .Leffects_8015e234
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    jal audio_effects_set_voice_volume
    sra $a0, $a0, 0x10
.Leffects_8015e234:
    lui $v0, %hi(audio_cue_work + 0x5c)
    lh $v0, %lo(audio_cue_work + 0x5c)($v0)
    lui $a0, %hi(audio_cue_work + 0x60)
    lh $a0, %lo(audio_cue_work + 0x60)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work + 0x50)
    lh $a2, %lo(audio_cue_work + 0x50)($a2)
    lui $a3, %hi(audio_cue_work + 0x58)
    lh $a3, %lo(audio_cue_work + 0x58)($a3)
    lui $v1, %hi(audio_cue_work + 0x68)
    lh $v1, %lo(audio_cue_work + 0x68)($v1)
    lui $t0, %hi(audio_cue_work + 0x6c)
    lh $t0, %lo(audio_cue_work + 0x6c)($t0)
    lui $t1, %hi(audio_cue_work + 0x70)
    lh $t1, %lo(audio_cue_work + 0x70)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x64)
    sh $a0, %lo(audio_cue_work + 0x64)($at)
    bne $v1, $v0, .Leffects_8015e2c0
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    jal audio_effects_set_voice_volume
    sra $a0, $a0, 0x10
.Leffects_8015e2c0:
    lui $v0, %hi(audio_cue_work + 0x34)
    lh $v0, %lo(audio_cue_work + 0x34)($v0)
    lui $a0, %hi(audio_cue_work + 0x38)
    lh $a0, %lo(audio_cue_work + 0x38)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work + 0x28)
    lh $a2, %lo(audio_cue_work + 0x28)($a2)
    lui $a3, %hi(audio_cue_work + 0x30)
    lh $a3, %lo(audio_cue_work + 0x30)($a3)
    lui $v1, %hi(audio_cue_work + 0x40)
    lh $v1, %lo(audio_cue_work + 0x40)($v1)
    lui $t0, %hi(audio_cue_work + 0x44)
    lh $t0, %lo(audio_cue_work + 0x44)($t0)
    lui $t1, %hi(audio_cue_work + 0x48)
    lh $t1, %lo(audio_cue_work + 0x48)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x3c)
    sh $a0, %lo(audio_cue_work + 0x3c)($at)
    bne $v1, $v0, .Leffects_8015e34c
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    jal audio_effects_set_voice_volume
    sra $a0, $a0, 0x10
.Leffects_8015e34c:
    lui $v0, %hi(audio_cue_work + 0xc)
    lh $v0, %lo(audio_cue_work + 0xc)($v0)
    lui $a0, %hi(audio_cue_work + 0x10)
    lh $a0, %lo(audio_cue_work + 0x10)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work)
    lh $a2, %lo(audio_cue_work)($a2)
    lui $a3, %hi(audio_cue_work + 0x8)
    lh $a3, %lo(audio_cue_work + 0x8)($a3)
    lui $v1, %hi(audio_cue_work + 0x18)
    lh $v1, %lo(audio_cue_work + 0x18)($v1)
    lui $t0, %hi(audio_cue_work + 0x1c)
    lh $t0, %lo(audio_cue_work + 0x1c)($t0)
    lui $t1, %hi(audio_cue_work + 0x20)
    lh $t1, %lo(audio_cue_work + 0x20)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x14)
    sh $a0, %lo(audio_cue_work + 0x14)($at)
    bne $v1, $v0, .Leffects_8015e3d8
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    jal audio_effects_set_voice_volume
    sra $a0, $a0, 0x10
.Leffects_8015e3d8:
    lui $v0, %hi(audio_cue_work + 0x10)
    lhu $v0, %lo(audio_cue_work + 0x10)($v0)
    lui $a0, %hi(audio_cue_work + 0xe0)
    lhu $a0, %lo(audio_cue_work + 0xe0)($a0)
    addiu $v1, $zero, -1
    lui $at, %hi(audio_cue_dispatch_prefix + 0x2)
    sb $v1, %lo(audio_cue_dispatch_prefix + 0x2)($at)
    lui $at, %hi(audio_cue_work + 0xe8)
    sh $v0, %lo(audio_cue_work + 0xe8)($at)
    lui $at, %hi(audio_cue_work + 0xe4)
    sh $a0, %lo(audio_cue_work + 0xe4)($at)
    j .Leffects_8015e980
    nop
.Leffects_8015e40c:
    lui $a0, %hi(audio_cue_work + 0xe8)
    lh $a0, %lo(audio_cue_work + 0xe8)($a0)
    lui $v0, %hi(audio_cue_work + 0x10)
    lh $v0, %lo(audio_cue_work + 0x10)($v0)
    nop
    beq $a0, $v0, .Leffects_8015e44c
    addiu $v0, $zero, 1
    lui $v1, %hi(audio_cue_work + 0xdc)
    lh $v1, %lo(audio_cue_work + 0xdc)($v1)
    nop
    beq $v1, $v0, .Leffects_8015e844
    slti $v0, $v1, 2
    bnez $v0, .Leffects_8015e70c
    addiu $v0, $zero, 2
    j .Leffects_8015e71c
    nop
.Leffects_8015e44c:
    lui $v1, %hi(audio_cue_work + 0xe4)
    lh $v1, %lo(audio_cue_work + 0xe4)($v1)
    lui $v0, %hi(audio_cue_work + 0xe0)
    lh $v0, %lo(audio_cue_work + 0xe0)($v0)
    nop
    slt $v0, $v0, $v1
    bnez $v0, .Leffects_8015e6d8
    sll $v0, $a0, 2
    lui $v1, %hi(audio_cue_work + 0xdc)
    lh $v1, %lo(audio_cue_work + 0xdc)($v1)
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_8015e5c4
    slti $v0, $v1, 2
    beqz $v0, .Leffects_8015e498
    nop
    beqz $v1, .Leffects_8015e650
    nop
    j .Leffects_8015e980
    nop
.Leffects_8015e498:
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_8015e538
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_8015e980
    nop
    lui $v0, %hi(audio_cue_work + 0x84)
    lh $v0, %lo(audio_cue_work + 0x84)($v0)
    lui $a0, %hi(audio_cue_work + 0x88)
    lh $a0, %lo(audio_cue_work + 0x88)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work + 0x78)
    lh $a2, %lo(audio_cue_work + 0x78)($a2)
    lui $a3, %hi(audio_cue_work + 0x80)
    lh $a3, %lo(audio_cue_work + 0x80)($a3)
    lui $v1, %hi(audio_cue_work + 0x90)
    lh $v1, %lo(audio_cue_work + 0x90)($v1)
    lui $t0, %hi(audio_cue_work + 0x94)
    lh $t0, %lo(audio_cue_work + 0x94)($t0)
    lui $t1, %hi(audio_cue_work + 0x98)
    lh $t1, %lo(audio_cue_work + 0x98)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x8c)
    sh $a0, %lo(audio_cue_work + 0x8c)($at)
    bne $v1, $v0, .Leffects_8015e538
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    jal audio_effects_set_voice_volume
    sra $a0, $a0, 0x10
.Leffects_8015e538:
    lui $v0, %hi(audio_cue_work + 0x5c)
    lh $v0, %lo(audio_cue_work + 0x5c)($v0)
    lui $a0, %hi(audio_cue_work + 0x60)
    lh $a0, %lo(audio_cue_work + 0x60)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work + 0x50)
    lh $a2, %lo(audio_cue_work + 0x50)($a2)
    lui $a3, %hi(audio_cue_work + 0x58)
    lh $a3, %lo(audio_cue_work + 0x58)($a3)
    lui $v1, %hi(audio_cue_work + 0x68)
    lh $v1, %lo(audio_cue_work + 0x68)($v1)
    lui $t0, %hi(audio_cue_work + 0x6c)
    lh $t0, %lo(audio_cue_work + 0x6c)($t0)
    lui $t1, %hi(audio_cue_work + 0x70)
    lh $t1, %lo(audio_cue_work + 0x70)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x64)
    sh $a0, %lo(audio_cue_work + 0x64)($at)
    bne $v1, $v0, .Leffects_8015e5c4
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    jal audio_effects_set_voice_volume
    sra $a0, $a0, 0x10
.Leffects_8015e5c4:
    lui $v0, %hi(audio_cue_work + 0x34)
    lh $v0, %lo(audio_cue_work + 0x34)($v0)
    lui $a0, %hi(audio_cue_work + 0x38)
    lh $a0, %lo(audio_cue_work + 0x38)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work + 0x28)
    lh $a2, %lo(audio_cue_work + 0x28)($a2)
    lui $a3, %hi(audio_cue_work + 0x30)
    lh $a3, %lo(audio_cue_work + 0x30)($a3)
    lui $v1, %hi(audio_cue_work + 0x40)
    lh $v1, %lo(audio_cue_work + 0x40)($v1)
    lui $t0, %hi(audio_cue_work + 0x44)
    lh $t0, %lo(audio_cue_work + 0x44)($t0)
    lui $t1, %hi(audio_cue_work + 0x48)
    lh $t1, %lo(audio_cue_work + 0x48)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x3c)
    sh $a0, %lo(audio_cue_work + 0x3c)($at)
    bne $v1, $v0, .Leffects_8015e650
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    jal audio_effects_set_voice_volume
    sra $a0, $a0, 0x10
.Leffects_8015e650:
    lui $v0, %hi(audio_cue_work + 0xc)
    lh $v0, %lo(audio_cue_work + 0xc)($v0)
    lui $a0, %hi(audio_cue_work + 0x10)
    lh $a0, %lo(audio_cue_work + 0x10)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work)
    lh $a2, %lo(audio_cue_work)($a2)
    lui $a3, %hi(audio_cue_work + 0x8)
    lh $a3, %lo(audio_cue_work + 0x8)($a3)
    lui $v1, %hi(audio_cue_work + 0x18)
    lh $v1, %lo(audio_cue_work + 0x18)($v1)
    lui $t0, %hi(audio_cue_work + 0x1c)
    lh $t0, %lo(audio_cue_work + 0x1c)($t0)
    lui $t1, %hi(audio_cue_work + 0x20)
    lh $t1, %lo(audio_cue_work + 0x20)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x14)
    sh $a0, %lo(audio_cue_work + 0x14)($at)
    bne $v1, $v0, .Leffects_8015e960
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd4)
    lh $a1, %lo(audio_cue_work + 0xd4)($a1)
    sra $a0, $a0, 0x10
    j .Leffects_8015e958
    addu $a2, $a1, $zero
.Leffects_8015e6d8:
    lui $at, %hi(audio_sequence_voice_globals + 0x5f0)
    addu $at, $at, $v0
    lw $v0, %lo(audio_sequence_voice_globals + 0x5f0)($at)
    nop
    bnez $v0, .Leffects_8015e980
    addiu $v0, $zero, 1
    lui $v1, %hi(audio_cue_work + 0xdc)
    lh $v1, %lo(audio_cue_work + 0xdc)($v1)
    nop
    beq $v1, $v0, .Leffects_8015e844
    slti $v0, $v1, 2
    beqz $v0, .Leffects_8015e71c
    addiu $v0, $zero, 2
.Leffects_8015e70c:
    beqz $v1, .Leffects_8015e8d0
    nop
    j .Leffects_8015e980
    nop
.Leffects_8015e71c:
    beq $v1, $v0, .Leffects_8015e7b8
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_8015e980
    nop
    lui $v0, %hi(audio_cue_work + 0x84)
    lh $v0, %lo(audio_cue_work + 0x84)($v0)
    lui $a0, %hi(audio_cue_work + 0x88)
    lh $a0, %lo(audio_cue_work + 0x88)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work + 0x78)
    lh $a2, %lo(audio_cue_work + 0x78)($a2)
    lui $a3, %hi(audio_cue_work + 0x80)
    lh $a3, %lo(audio_cue_work + 0x80)($a3)
    lui $v1, %hi(audio_cue_work + 0x90)
    lh $v1, %lo(audio_cue_work + 0x90)($v1)
    lui $t0, %hi(audio_cue_work + 0x94)
    lh $t0, %lo(audio_cue_work + 0x94)($t0)
    lui $t1, %hi(audio_cue_work + 0x98)
    lh $t1, %lo(audio_cue_work + 0x98)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x8c)
    sh $a0, %lo(audio_cue_work + 0x8c)($at)
    bne $v1, $v0, .Leffects_8015e7b8
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    jal audio_effects_set_voice_volume
    sra $a0, $a0, 0x10
.Leffects_8015e7b8:
    lui $v0, %hi(audio_cue_work + 0x5c)
    lh $v0, %lo(audio_cue_work + 0x5c)($v0)
    lui $a0, %hi(audio_cue_work + 0x60)
    lh $a0, %lo(audio_cue_work + 0x60)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work + 0x50)
    lh $a2, %lo(audio_cue_work + 0x50)($a2)
    lui $a3, %hi(audio_cue_work + 0x58)
    lh $a3, %lo(audio_cue_work + 0x58)($a3)
    lui $v1, %hi(audio_cue_work + 0x68)
    lh $v1, %lo(audio_cue_work + 0x68)($v1)
    lui $t0, %hi(audio_cue_work + 0x6c)
    lh $t0, %lo(audio_cue_work + 0x6c)($t0)
    lui $t1, %hi(audio_cue_work + 0x70)
    lh $t1, %lo(audio_cue_work + 0x70)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x64)
    sh $a0, %lo(audio_cue_work + 0x64)($at)
    bne $v1, $v0, .Leffects_8015e844
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    jal audio_effects_set_voice_volume
    sra $a0, $a0, 0x10
.Leffects_8015e844:
    lui $v0, %hi(audio_cue_work + 0x34)
    lh $v0, %lo(audio_cue_work + 0x34)($v0)
    lui $a0, %hi(audio_cue_work + 0x38)
    lh $a0, %lo(audio_cue_work + 0x38)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work + 0x28)
    lh $a2, %lo(audio_cue_work + 0x28)($a2)
    lui $a3, %hi(audio_cue_work + 0x30)
    lh $a3, %lo(audio_cue_work + 0x30)($a3)
    lui $v1, %hi(audio_cue_work + 0x40)
    lh $v1, %lo(audio_cue_work + 0x40)($v1)
    lui $t0, %hi(audio_cue_work + 0x44)
    lh $t0, %lo(audio_cue_work + 0x44)($t0)
    lui $t1, %hi(audio_cue_work + 0x48)
    lh $t1, %lo(audio_cue_work + 0x48)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x3c)
    sh $a0, %lo(audio_cue_work + 0x3c)($at)
    bne $v1, $v0, .Leffects_8015e8d0
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    jal audio_effects_set_voice_volume
    sra $a0, $a0, 0x10
.Leffects_8015e8d0:
    lui $v0, %hi(audio_cue_work + 0xc)
    lh $v0, %lo(audio_cue_work + 0xc)($v0)
    lui $a0, %hi(audio_cue_work + 0x10)
    lh $a0, %lo(audio_cue_work + 0x10)($a0)
    lui $a1, %hi(audio_cue_work + 0x4)
    lh $a1, %lo(audio_cue_work + 0x4)($a1)
    lui $a2, %hi(audio_cue_work)
    lh $a2, %lo(audio_cue_work)($a2)
    lui $a3, %hi(audio_cue_work + 0x8)
    lh $a3, %lo(audio_cue_work + 0x8)($a3)
    lui $v1, %hi(audio_cue_work + 0x18)
    lh $v1, %lo(audio_cue_work + 0x18)($v1)
    lui $t0, %hi(audio_cue_work + 0x1c)
    lh $t0, %lo(audio_cue_work + 0x1c)($t0)
    lui $t1, %hi(audio_cue_work + 0x20)
    lh $t1, %lo(audio_cue_work + 0x20)($t1)
    sw $v0, 0x10($sp)
    sw $v1, 0x14($sp)
    sw $t0, 0x18($sp)
    jal audio_effects_key_on
    sw $t1, 0x1c($sp)
    addu $a0, $v0, $zero
    lui $v1, %hi(audio_cue_work + 0xec)
    lh $v1, %lo(audio_cue_work + 0xec)($v1)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_cue_work + 0x14)
    sh $a0, %lo(audio_cue_work + 0x14)($at)
    bne $v1, $v0, .Leffects_8015e960
    sll $a0, $a0, 0x10
    lui $a1, %hi(audio_cue_work + 0xd0)
    lh $a1, %lo(audio_cue_work + 0xd0)($a1)
    lui $a2, %hi(audio_cue_work + 0xd4)
    lh $a2, %lo(audio_cue_work + 0xd4)($a2)
    sra $a0, $a0, 0x10
.Leffects_8015e958:
    jal audio_effects_set_voice_volume
    nop
.Leffects_8015e960:
    lui $v0, %hi(audio_cue_work + 0x10)
    lhu $v0, %lo(audio_cue_work + 0x10)($v0)
    lui $v1, %hi(audio_cue_work + 0xe0)
    lhu $v1, %lo(audio_cue_work + 0xe0)($v1)
    lui $at, %hi(audio_cue_work + 0xe8)
    sh $v0, %lo(audio_cue_work + 0xe8)($at)
    lui $at, %hi(audio_cue_work + 0xe4)
    sh $v1, %lo(audio_cue_work + 0xe4)($at)
.Leffects_8015e980:
    lw $ra, 0x24($sp)
    lw $s0, 0x20($sp)
    addiu $sp, $sp, 0x28
    jr $ra
    nop
.end audio_cue_dispatch

.section .text
.balign 4
# exe/slus_004_22@0x8015E994..0x8015EFAC
.globl audio_cue_handler_0
.ent audio_cue_handler_0
audio_cue_handler_0:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $at, %hi(audio_cue_work + 0x4)
    sh $zero, %lo(audio_cue_work + 0x4)($at)
    sll $a1, $v0, 2
    lui $at, %hi(audio_loader_auxiliary)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary)($at)
    lui $at, %hi(audio_cue_work + 0xd8)
    sh $v0, %lo(audio_cue_work + 0xd8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1)
    addu $at, $at, $a1
    lbu $v1, %lo(audio_loader_auxiliary + 0x1)($at)
    nop
    andi $v1, $v1, 0x7f
    andi $v1, $v1, 0xff
    lui $at, %hi(audio_cue_work)
    sh $v1, %lo(audio_cue_work)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x1)($at)
    nop
    andi $v0, $v0, 0x80
    lui $at, %hi(audio_cue_work + 0xec)
    sh $v0, %lo(audio_cue_work + 0xec)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x2)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x2)($at)
    lui $t4, %hi(audio_loader_header_pointers)
    lw $t4, %lo(audio_loader_header_pointers)($t4)
    srl $a2, $v0, 4
    lui $at, %hi(audio_cue_work + 0x8)
    sh $a2, %lo(audio_cue_work + 0x8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x2)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x2)($at)
    sll $v1, $v1, 1
    andi $v0, $v0, 0xf
    lui $at, %hi(audio_cue_work + 0xe0)
    sh $v0, %lo(audio_cue_work + 0xe0)($at)
    sll $v0, $a2, 1
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v0, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $v1
    lhu $v1, %lo(audio_cue_lookup)($at)
    addiu $v0, $v0, 0x820
    addu $v1, $v1, $v0
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v1, %lo(audio_cue_work + 0x104)($at)
    andi $v1, $v1, 0xffff
    addu $v1, $v1, $t4
    lbu $t3, 6($v1)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $t3, %lo(audio_cue_work + 0xc)($at)
    lbu $t2, 5($v1)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $t2, %lo(audio_cue_work + 0x18)($at)
    lbu $v0, 3($v1)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v0, %lo(audio_cue_work + 0x24)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v0, %lo(audio_cue_work + 0x1c)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v0, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x3)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x3)($at)
    lui $t0, %hi(audio_cue_lookup)
    addiu $t0, $t0, %lo(audio_cue_lookup)
    andi $v0, $v0, 0x1f
    andi $a3, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x10)
    sh $a3, %lo(audio_cue_work + 0x10)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x3)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x3)($at)
    lui $t1, %hi(audio_cue_lookup + 0x8)
    addiu $t1, $t1, %lo(audio_cue_lookup + 0x8)
    andi $v0, $v0, 0x60
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $v0, %lo(audio_cue_work + 0xdc)($at)
    sll $v0, $v0, 0x10
    beqz $v0, .Leffects_8015eddc
    addiu $sp, $sp, -0x18
    sra $a0, $v0, 0x15
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $a0, %lo(audio_cue_work + 0xdc)($at)
    addu $v1, $a0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_8015ebe4
    slti $v0, $v1, 3
    beqz $v0, .Leffects_8015eb2c
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_8015ece0
    nop
    j .Leffects_8015eddc
    nop
.Leffects_8015eb2c:
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_8015eddc
    nop
    lui $at, %hi(audio_loader_auxiliary + 0x1)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x1)($at)
    addu $v1, $a2, $a0
    lui $at, %hi(audio_cue_work + 0x80)
    sh $v1, %lo(audio_cue_work + 0x80)($at)
    sll $v1, $v1, 1
    addu $v1, $v1, $t1
    lhu $v1, 0($v1)
    lui $at, %hi(audio_cue_work + 0x84)
    sh $t3, %lo(audio_cue_work + 0x84)($at)
    lui $at, %hi(audio_cue_work + 0x90)
    sh $t2, %lo(audio_cue_work + 0x90)($at)
    andi $v0, $v0, 0x7f
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x78)
    sh $v0, %lo(audio_cue_work + 0x78)($at)
    sll $v0, $v0, 1
    addu $v0, $v0, $t0
    lhu $v0, 0($v0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $t4
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x94)
    sh $v1, %lo(audio_cue_work + 0x94)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x98)
    sh $v1, %lo(audio_cue_work + 0x98)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a3, 3
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x9c)
    sh $v1, %lo(audio_cue_work + 0x9c)($at)
    bnez $v0, .Leffects_8015ebe4
    addiu $v0, $a3, -0x15
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
.Leffects_8015ebe4:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x1)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x1)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x50)
    sh $a0, %lo(audio_cue_work + 0x50)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x58)
    sh $v0, %lo(audio_cue_work + 0x58)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers)
    lw $v1, %lo(audio_loader_header_pointers)($v1)
    lui $at, %hi(audio_cue_work + 0x5c)
    sh $a1, %lo(audio_cue_work + 0x5c)($at)
    lui $at, %hi(audio_cue_work + 0x68)
    sh $a2, %lo(audio_cue_work + 0x68)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x6c)
    sh $v1, %lo(audio_cue_work + 0x6c)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x70)
    sh $v1, %lo(audio_cue_work + 0x70)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 2
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x74)
    sh $v1, %lo(audio_cue_work + 0x74)($at)
    bnez $v0, .Leffects_8015ece0
    addiu $v0, $a0, -0x16
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
.Leffects_8015ece0:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x1)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x1)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x28)
    sh $a0, %lo(audio_cue_work + 0x28)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x30)
    sh $v0, %lo(audio_cue_work + 0x30)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers)
    lw $v1, %lo(audio_loader_header_pointers)($v1)
    lui $at, %hi(audio_cue_work + 0x34)
    sh $a1, %lo(audio_cue_work + 0x34)($at)
    lui $at, %hi(audio_cue_work + 0x40)
    sh $a2, %lo(audio_cue_work + 0x40)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x44)
    sh $v1, %lo(audio_cue_work + 0x44)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x48)
    sh $v1, %lo(audio_cue_work + 0x48)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x4c)
    sh $v1, %lo(audio_cue_work + 0x4c)($at)
    bnez $v0, .Leffects_8015eddc
    addiu $v0, $a0, -0x17
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
.Leffects_8015eddc:
    lui $v0, %hi(audio_cue_work + 0xd8)
    lh $v0, %lo(audio_cue_work + 0xd8)($v0)
    nop
    beqz $v0, .Leffects_8015efa0
    addu $v1, $v0, $zero
    andi $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    addu $v1, $v0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_8015eeb0
    slti $v0, $v1, 3
    beqz $v0, .Leffects_8015ee24
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_8015ee38
    nop
    j .Leffects_8015efa0
    nop
.Leffects_8015ee24:
    addiu $v0, $zero, 3
    beq $v1, $v0, .Leffects_8015ef28
    nop
    j .Leffects_8015efa0
    nop
.Leffects_8015ee38:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x4)
    lw $v1, %lo(audio_loader_header_pointers + 0x4)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7f)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x7f)($at)
    j .Leffects_8015ef98
    nop
.Leffects_8015eeb0:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x8)
    lw $v1, %lo(audio_loader_header_pointers + 0x8)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xfb)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0xfb)($at)
    j .Leffects_8015ef98
    nop
.Leffects_8015ef28:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0xc)
    lw $v1, %lo(audio_loader_header_pointers + 0xc)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x177)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x177)($at)
.Leffects_8015ef98:
    lui $at, %hi(audio_cue_work + 0x10)
    sh $v0, %lo(audio_cue_work + 0x10)($at)
.Leffects_8015efa0:
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cue_handler_0

.section .text
.balign 4
# exe/slus_004_22@0x8015EFAC..0x8015F5C8
.globl audio_cue_handler_1
.ent audio_cue_handler_1
audio_cue_handler_1:
    lui $v1, %hi(audio_cue_work + 0xfc)
    lhu $v1, %lo(audio_cue_work + 0xfc)($v1)
    addiu $v0, $zero, 1
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    sll $a1, $v1, 2
    lui $at, %hi(audio_loader_auxiliary + 0x7c)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x7c)($at)
    lui $at, %hi(audio_cue_work + 0xd8)
    sh $v0, %lo(audio_cue_work + 0xd8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7d)
    addu $at, $at, $a1
    lbu $v1, %lo(audio_loader_auxiliary + 0x7d)($at)
    nop
    andi $v1, $v1, 0x7f
    andi $v1, $v1, 0xff
    lui $at, %hi(audio_cue_work)
    sh $v1, %lo(audio_cue_work)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7d)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x7d)($at)
    nop
    andi $v0, $v0, 0x80
    lui $at, %hi(audio_cue_work + 0xec)
    sh $v0, %lo(audio_cue_work + 0xec)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7e)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x7e)($at)
    lui $t4, %hi(audio_loader_header_pointers + 0x4)
    lw $t4, %lo(audio_loader_header_pointers + 0x4)($t4)
    srl $a2, $v0, 4
    lui $at, %hi(audio_cue_work + 0x8)
    sh $a2, %lo(audio_cue_work + 0x8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7e)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x7e)($at)
    sll $v1, $v1, 1
    andi $v0, $v0, 0xf
    lui $at, %hi(audio_cue_work + 0xe0)
    sh $v0, %lo(audio_cue_work + 0xe0)($at)
    sll $v0, $a2, 1
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v0, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $v1
    lhu $v1, %lo(audio_cue_lookup)($at)
    addiu $v0, $v0, 0x820
    addu $v1, $v1, $v0
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v1, %lo(audio_cue_work + 0x104)($at)
    andi $v1, $v1, 0xffff
    addu $v1, $v1, $t4
    lbu $t3, 6($v1)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $t3, %lo(audio_cue_work + 0xc)($at)
    lbu $t2, 5($v1)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $t2, %lo(audio_cue_work + 0x18)($at)
    lbu $v0, 3($v1)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v0, %lo(audio_cue_work + 0x24)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v0, %lo(audio_cue_work + 0x1c)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v0, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7f)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x7f)($at)
    lui $t0, %hi(audio_cue_lookup)
    addiu $t0, $t0, %lo(audio_cue_lookup)
    andi $v0, $v0, 0x1f
    andi $a3, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x10)
    sh $a3, %lo(audio_cue_work + 0x10)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7f)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x7f)($at)
    lui $t1, %hi(audio_cue_lookup + 0x8)
    addiu $t1, $t1, %lo(audio_cue_lookup + 0x8)
    andi $v0, $v0, 0x60
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $v0, %lo(audio_cue_work + 0xdc)($at)
    sll $v0, $v0, 0x10
    beqz $v0, .Leffects_8015f3f8
    addiu $sp, $sp, -0x18
    sra $a0, $v0, 0x15
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $a0, %lo(audio_cue_work + 0xdc)($at)
    addu $v1, $a0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_8015f200
    slti $v0, $v1, 3
    beqz $v0, .Leffects_8015f148
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_8015f2fc
    nop
    j .Leffects_8015f3f8
    nop
.Leffects_8015f148:
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_8015f3f8
    nop
    lui $at, %hi(audio_loader_auxiliary + 0x7d)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x7d)($at)
    addu $v1, $a2, $a0
    lui $at, %hi(audio_cue_work + 0x80)
    sh $v1, %lo(audio_cue_work + 0x80)($at)
    sll $v1, $v1, 1
    addu $v1, $v1, $t1
    lhu $v1, 0($v1)
    lui $at, %hi(audio_cue_work + 0x84)
    sh $t3, %lo(audio_cue_work + 0x84)($at)
    lui $at, %hi(audio_cue_work + 0x90)
    sh $t2, %lo(audio_cue_work + 0x90)($at)
    andi $v0, $v0, 0x7f
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x78)
    sh $v0, %lo(audio_cue_work + 0x78)($at)
    sll $v0, $v0, 1
    addu $v0, $v0, $t0
    lhu $v0, 0($v0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $t4
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x94)
    sh $v1, %lo(audio_cue_work + 0x94)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x98)
    sh $v1, %lo(audio_cue_work + 0x98)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a3, 3
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x9c)
    sh $v1, %lo(audio_cue_work + 0x9c)($at)
    bnez $v0, .Leffects_8015f200
    addiu $v0, $a3, -0x15
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
.Leffects_8015f200:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x7d)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x7d)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x50)
    sh $a0, %lo(audio_cue_work + 0x50)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x58)
    sh $v0, %lo(audio_cue_work + 0x58)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0x4)
    lw $v1, %lo(audio_loader_header_pointers + 0x4)($v1)
    lui $at, %hi(audio_cue_work + 0x5c)
    sh $a1, %lo(audio_cue_work + 0x5c)($at)
    lui $at, %hi(audio_cue_work + 0x68)
    sh $a2, %lo(audio_cue_work + 0x68)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x6c)
    sh $v1, %lo(audio_cue_work + 0x6c)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x70)
    sh $v1, %lo(audio_cue_work + 0x70)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 2
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x74)
    sh $v1, %lo(audio_cue_work + 0x74)($at)
    bnez $v0, .Leffects_8015f2fc
    addiu $v0, $a0, -0x16
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
.Leffects_8015f2fc:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x7d)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x7d)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x28)
    sh $a0, %lo(audio_cue_work + 0x28)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x30)
    sh $v0, %lo(audio_cue_work + 0x30)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0x4)
    lw $v1, %lo(audio_loader_header_pointers + 0x4)($v1)
    lui $at, %hi(audio_cue_work + 0x34)
    sh $a1, %lo(audio_cue_work + 0x34)($at)
    lui $at, %hi(audio_cue_work + 0x40)
    sh $a2, %lo(audio_cue_work + 0x40)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x44)
    sh $v1, %lo(audio_cue_work + 0x44)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x48)
    sh $v1, %lo(audio_cue_work + 0x48)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x4c)
    sh $v1, %lo(audio_cue_work + 0x4c)($at)
    bnez $v0, .Leffects_8015f3f8
    addiu $v0, $a0, -0x17
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
.Leffects_8015f3f8:
    lui $v0, %hi(audio_cue_work + 0xd8)
    lh $v0, %lo(audio_cue_work + 0xd8)($v0)
    nop
    beqz $v0, .Leffects_8015f5bc
    addu $v1, $v0, $zero
    andi $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    addu $v1, $v0, $zero
    addiu $v0, $zero, 3
    beq $v1, $v0, .Leffects_8015f4cc
    slti $v0, $v1, 4
    beqz $v0, .Leffects_8015f440
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_8015f454
    nop
    j .Leffects_8015f5bc
    nop
.Leffects_8015f440:
    addiu $v0, $zero, 4
    beq $v1, $v0, .Leffects_8015f544
    nop
    j .Leffects_8015f5bc
    nop
.Leffects_8015f454:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x8)
    lw $v1, %lo(audio_loader_header_pointers + 0x8)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xfb)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0xfb)($at)
    j .Leffects_8015f5b4
    nop
.Leffects_8015f4cc:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0xc)
    lw $v1, %lo(audio_loader_header_pointers + 0xc)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x177)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x177)($at)
    j .Leffects_8015f5b4
    nop
.Leffects_8015f544:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x10)
    lw $v1, %lo(audio_loader_header_pointers + 0x10)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1f3)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x1f3)($at)
.Leffects_8015f5b4:
    lui $at, %hi(audio_cue_work + 0x10)
    sh $v0, %lo(audio_cue_work + 0x10)($at)
.Leffects_8015f5bc:
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cue_handler_1

.section .text
.balign 4
# exe/slus_004_22@0x8015F5C8..0x8015FBE4
.globl audio_cue_handler_2
.ent audio_cue_handler_2
audio_cue_handler_2:
    lui $v1, %hi(audio_cue_work + 0xfc)
    lhu $v1, %lo(audio_cue_work + 0xfc)($v1)
    addiu $v0, $zero, 2
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    sll $a1, $v1, 2
    lui $at, %hi(audio_loader_auxiliary + 0xf8)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0xf8)($at)
    lui $at, %hi(audio_cue_work + 0xd8)
    sh $v0, %lo(audio_cue_work + 0xd8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xf9)
    addu $at, $at, $a1
    lbu $v1, %lo(audio_loader_auxiliary + 0xf9)($at)
    nop
    andi $v1, $v1, 0x7f
    andi $v1, $v1, 0xff
    lui $at, %hi(audio_cue_work)
    sh $v1, %lo(audio_cue_work)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xf9)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0xf9)($at)
    nop
    andi $v0, $v0, 0x80
    lui $at, %hi(audio_cue_work + 0xec)
    sh $v0, %lo(audio_cue_work + 0xec)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xfa)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0xfa)($at)
    lui $t4, %hi(audio_loader_header_pointers + 0x8)
    lw $t4, %lo(audio_loader_header_pointers + 0x8)($t4)
    srl $a2, $v0, 4
    lui $at, %hi(audio_cue_work + 0x8)
    sh $a2, %lo(audio_cue_work + 0x8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xfa)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0xfa)($at)
    sll $v1, $v1, 1
    andi $v0, $v0, 0xf
    lui $at, %hi(audio_cue_work + 0xe0)
    sh $v0, %lo(audio_cue_work + 0xe0)($at)
    sll $v0, $a2, 1
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v0, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $v1
    lhu $v1, %lo(audio_cue_lookup)($at)
    addiu $v0, $v0, 0x820
    addu $v1, $v1, $v0
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v1, %lo(audio_cue_work + 0x104)($at)
    andi $v1, $v1, 0xffff
    addu $v1, $v1, $t4
    lbu $t3, 6($v1)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $t3, %lo(audio_cue_work + 0xc)($at)
    lbu $t2, 5($v1)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $t2, %lo(audio_cue_work + 0x18)($at)
    lbu $v0, 3($v1)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v0, %lo(audio_cue_work + 0x24)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v0, %lo(audio_cue_work + 0x1c)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v0, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xfb)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0xfb)($at)
    lui $t0, %hi(audio_cue_lookup)
    addiu $t0, $t0, %lo(audio_cue_lookup)
    andi $v0, $v0, 0x1f
    andi $a3, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x10)
    sh $a3, %lo(audio_cue_work + 0x10)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xfb)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0xfb)($at)
    lui $t1, %hi(audio_cue_lookup + 0x8)
    addiu $t1, $t1, %lo(audio_cue_lookup + 0x8)
    andi $v0, $v0, 0x60
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $v0, %lo(audio_cue_work + 0xdc)($at)
    sll $v0, $v0, 0x10
    beqz $v0, .Leffects_8015fa14
    addiu $sp, $sp, -0x18
    sra $a0, $v0, 0x15
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $a0, %lo(audio_cue_work + 0xdc)($at)
    addu $v1, $a0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_8015f81c
    slti $v0, $v1, 3
    beqz $v0, .Leffects_8015f764
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_8015f918
    nop
    j .Leffects_8015fa14
    nop
.Leffects_8015f764:
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_8015fa14
    nop
    lui $at, %hi(audio_loader_auxiliary + 0xf9)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0xf9)($at)
    addu $v1, $a2, $a0
    lui $at, %hi(audio_cue_work + 0x80)
    sh $v1, %lo(audio_cue_work + 0x80)($at)
    sll $v1, $v1, 1
    addu $v1, $v1, $t1
    lhu $v1, 0($v1)
    lui $at, %hi(audio_cue_work + 0x84)
    sh $t3, %lo(audio_cue_work + 0x84)($at)
    lui $at, %hi(audio_cue_work + 0x90)
    sh $t2, %lo(audio_cue_work + 0x90)($at)
    andi $v0, $v0, 0x7f
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x78)
    sh $v0, %lo(audio_cue_work + 0x78)($at)
    sll $v0, $v0, 1
    addu $v0, $v0, $t0
    lhu $v0, 0($v0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $t4
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x94)
    sh $v1, %lo(audio_cue_work + 0x94)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x98)
    sh $v1, %lo(audio_cue_work + 0x98)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a3, 3
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x9c)
    sh $v1, %lo(audio_cue_work + 0x9c)($at)
    bnez $v0, .Leffects_8015f81c
    addiu $v0, $a3, -0x15
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
.Leffects_8015f81c:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0xf9)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0xf9)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x50)
    sh $a0, %lo(audio_cue_work + 0x50)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x58)
    sh $v0, %lo(audio_cue_work + 0x58)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0x8)
    lw $v1, %lo(audio_loader_header_pointers + 0x8)($v1)
    lui $at, %hi(audio_cue_work + 0x5c)
    sh $a1, %lo(audio_cue_work + 0x5c)($at)
    lui $at, %hi(audio_cue_work + 0x68)
    sh $a2, %lo(audio_cue_work + 0x68)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x6c)
    sh $v1, %lo(audio_cue_work + 0x6c)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x70)
    sh $v1, %lo(audio_cue_work + 0x70)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 2
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x74)
    sh $v1, %lo(audio_cue_work + 0x74)($at)
    bnez $v0, .Leffects_8015f918
    addiu $v0, $a0, -0x16
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
.Leffects_8015f918:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0xf9)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0xf9)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x28)
    sh $a0, %lo(audio_cue_work + 0x28)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x30)
    sh $v0, %lo(audio_cue_work + 0x30)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0x8)
    lw $v1, %lo(audio_loader_header_pointers + 0x8)($v1)
    lui $at, %hi(audio_cue_work + 0x34)
    sh $a1, %lo(audio_cue_work + 0x34)($at)
    lui $at, %hi(audio_cue_work + 0x40)
    sh $a2, %lo(audio_cue_work + 0x40)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x44)
    sh $v1, %lo(audio_cue_work + 0x44)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x48)
    sh $v1, %lo(audio_cue_work + 0x48)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x4c)
    sh $v1, %lo(audio_cue_work + 0x4c)($at)
    bnez $v0, .Leffects_8015fa14
    addiu $v0, $a0, -0x17
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
.Leffects_8015fa14:
    lui $v0, %hi(audio_cue_work + 0xd8)
    lh $v0, %lo(audio_cue_work + 0xd8)($v0)
    nop
    beqz $v0, .Leffects_8015fbd8
    addu $v1, $v0, $zero
    andi $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    addu $v1, $v0, $zero
    addiu $v0, $zero, 3
    beq $v1, $v0, .Leffects_8015fae8
    slti $v0, $v1, 4
    beqz $v0, .Leffects_8015fa5c
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_8015fa70
    nop
    j .Leffects_8015fbd8
    nop
.Leffects_8015fa5c:
    addiu $v0, $zero, 4
    beq $v1, $v0, .Leffects_8015fb60
    nop
    j .Leffects_8015fbd8
    nop
.Leffects_8015fa70:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x4)
    lw $v1, %lo(audio_loader_header_pointers + 0x4)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7f)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x7f)($at)
    j .Leffects_8015fbd0
    nop
.Leffects_8015fae8:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0xc)
    lw $v1, %lo(audio_loader_header_pointers + 0xc)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x177)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x177)($at)
    j .Leffects_8015fbd0
    nop
.Leffects_8015fb60:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x10)
    lw $v1, %lo(audio_loader_header_pointers + 0x10)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1f3)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x1f3)($at)
.Leffects_8015fbd0:
    lui $at, %hi(audio_cue_work + 0x10)
    sh $v0, %lo(audio_cue_work + 0x10)($at)
.Leffects_8015fbd8:
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cue_handler_2

.section .text
.balign 4
# exe/slus_004_22@0x8015FBE4..0x80160200
.globl audio_cue_handler_3
.ent audio_cue_handler_3
audio_cue_handler_3:
    lui $v1, %hi(audio_cue_work + 0xfc)
    lhu $v1, %lo(audio_cue_work + 0xfc)($v1)
    addiu $v0, $zero, 3
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    sll $a1, $v1, 2
    lui $at, %hi(audio_loader_auxiliary + 0x174)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x174)($at)
    lui $at, %hi(audio_cue_work + 0xd8)
    sh $v0, %lo(audio_cue_work + 0xd8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x175)
    addu $at, $at, $a1
    lbu $v1, %lo(audio_loader_auxiliary + 0x175)($at)
    nop
    andi $v1, $v1, 0x7f
    andi $v1, $v1, 0xff
    lui $at, %hi(audio_cue_work)
    sh $v1, %lo(audio_cue_work)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x175)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x175)($at)
    nop
    andi $v0, $v0, 0x80
    lui $at, %hi(audio_cue_work + 0xec)
    sh $v0, %lo(audio_cue_work + 0xec)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x176)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x176)($at)
    lui $t4, %hi(audio_loader_header_pointers + 0xc)
    lw $t4, %lo(audio_loader_header_pointers + 0xc)($t4)
    srl $a2, $v0, 4
    lui $at, %hi(audio_cue_work + 0x8)
    sh $a2, %lo(audio_cue_work + 0x8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x176)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x176)($at)
    sll $v1, $v1, 1
    andi $v0, $v0, 0xf
    lui $at, %hi(audio_cue_work + 0xe0)
    sh $v0, %lo(audio_cue_work + 0xe0)($at)
    sll $v0, $a2, 1
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v0, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $v1
    lhu $v1, %lo(audio_cue_lookup)($at)
    addiu $v0, $v0, 0x820
    addu $v1, $v1, $v0
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v1, %lo(audio_cue_work + 0x104)($at)
    andi $v1, $v1, 0xffff
    addu $v1, $v1, $t4
    lbu $t3, 6($v1)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $t3, %lo(audio_cue_work + 0xc)($at)
    lbu $t2, 5($v1)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $t2, %lo(audio_cue_work + 0x18)($at)
    lbu $v0, 3($v1)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v0, %lo(audio_cue_work + 0x24)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v0, %lo(audio_cue_work + 0x1c)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v0, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x177)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x177)($at)
    lui $t0, %hi(audio_cue_lookup)
    addiu $t0, $t0, %lo(audio_cue_lookup)
    andi $v0, $v0, 0x1f
    andi $a3, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x10)
    sh $a3, %lo(audio_cue_work + 0x10)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x177)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x177)($at)
    lui $t1, %hi(audio_cue_lookup + 0x8)
    addiu $t1, $t1, %lo(audio_cue_lookup + 0x8)
    andi $v0, $v0, 0x60
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $v0, %lo(audio_cue_work + 0xdc)($at)
    sll $v0, $v0, 0x10
    beqz $v0, .Leffects_80160030
    addiu $sp, $sp, -0x18
    sra $a0, $v0, 0x15
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $a0, %lo(audio_cue_work + 0xdc)($at)
    addu $v1, $a0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_8015fe38
    slti $v0, $v1, 3
    beqz $v0, .Leffects_8015fd80
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_8015ff34
    nop
    j .Leffects_80160030
    nop
.Leffects_8015fd80:
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_80160030
    nop
    lui $at, %hi(audio_loader_auxiliary + 0x175)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x175)($at)
    addu $v1, $a2, $a0
    lui $at, %hi(audio_cue_work + 0x80)
    sh $v1, %lo(audio_cue_work + 0x80)($at)
    sll $v1, $v1, 1
    addu $v1, $v1, $t1
    lhu $v1, 0($v1)
    lui $at, %hi(audio_cue_work + 0x84)
    sh $t3, %lo(audio_cue_work + 0x84)($at)
    lui $at, %hi(audio_cue_work + 0x90)
    sh $t2, %lo(audio_cue_work + 0x90)($at)
    andi $v0, $v0, 0x7f
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x78)
    sh $v0, %lo(audio_cue_work + 0x78)($at)
    sll $v0, $v0, 1
    addu $v0, $v0, $t0
    lhu $v0, 0($v0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $t4
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x94)
    sh $v1, %lo(audio_cue_work + 0x94)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x98)
    sh $v1, %lo(audio_cue_work + 0x98)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a3, 3
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x9c)
    sh $v1, %lo(audio_cue_work + 0x9c)($at)
    bnez $v0, .Leffects_8015fe38
    addiu $v0, $a3, -0x15
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
.Leffects_8015fe38:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x175)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x175)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x50)
    sh $a0, %lo(audio_cue_work + 0x50)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x58)
    sh $v0, %lo(audio_cue_work + 0x58)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0xc)
    lw $v1, %lo(audio_loader_header_pointers + 0xc)($v1)
    lui $at, %hi(audio_cue_work + 0x5c)
    sh $a1, %lo(audio_cue_work + 0x5c)($at)
    lui $at, %hi(audio_cue_work + 0x68)
    sh $a2, %lo(audio_cue_work + 0x68)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x6c)
    sh $v1, %lo(audio_cue_work + 0x6c)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x70)
    sh $v1, %lo(audio_cue_work + 0x70)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 2
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x74)
    sh $v1, %lo(audio_cue_work + 0x74)($at)
    bnez $v0, .Leffects_8015ff34
    addiu $v0, $a0, -0x16
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
.Leffects_8015ff34:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x175)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x175)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x28)
    sh $a0, %lo(audio_cue_work + 0x28)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x30)
    sh $v0, %lo(audio_cue_work + 0x30)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0xc)
    lw $v1, %lo(audio_loader_header_pointers + 0xc)($v1)
    lui $at, %hi(audio_cue_work + 0x34)
    sh $a1, %lo(audio_cue_work + 0x34)($at)
    lui $at, %hi(audio_cue_work + 0x40)
    sh $a2, %lo(audio_cue_work + 0x40)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x44)
    sh $v1, %lo(audio_cue_work + 0x44)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x48)
    sh $v1, %lo(audio_cue_work + 0x48)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x4c)
    sh $v1, %lo(audio_cue_work + 0x4c)($at)
    bnez $v0, .Leffects_80160030
    addiu $v0, $a0, -0x17
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
.Leffects_80160030:
    lui $v0, %hi(audio_cue_work + 0xd8)
    lh $v0, %lo(audio_cue_work + 0xd8)($v0)
    nop
    beqz $v0, .Leffects_801601f4
    addu $v1, $v0, $zero
    andi $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    addu $v1, $v0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_80160104
    slti $v0, $v1, 3
    beqz $v0, .Leffects_80160078
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_8016008c
    nop
    j .Leffects_801601f4
    nop
.Leffects_80160078:
    addiu $v0, $zero, 4
    beq $v1, $v0, .Leffects_8016017c
    nop
    j .Leffects_801601f4
    nop
.Leffects_8016008c:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x4)
    lw $v1, %lo(audio_loader_header_pointers + 0x4)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7f)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x7f)($at)
    j .Leffects_801601ec
    nop
.Leffects_80160104:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x8)
    lw $v1, %lo(audio_loader_header_pointers + 0x8)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xfb)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0xfb)($at)
    j .Leffects_801601ec
    nop
.Leffects_8016017c:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x10)
    lw $v1, %lo(audio_loader_header_pointers + 0x10)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1f3)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x1f3)($at)
.Leffects_801601ec:
    lui $at, %hi(audio_cue_work + 0x10)
    sh $v0, %lo(audio_cue_work + 0x10)($at)
.Leffects_801601f4:
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cue_handler_3

.section .text
.balign 4
# exe/slus_004_22@0x80160200..0x8016081C
.globl audio_cue_handler_4
.ent audio_cue_handler_4
audio_cue_handler_4:
    lui $v1, %hi(audio_cue_work + 0xfc)
    lhu $v1, %lo(audio_cue_work + 0xfc)($v1)
    addiu $v0, $zero, 4
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    sll $a1, $v1, 2
    lui $at, %hi(audio_loader_auxiliary + 0x1f0)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x1f0)($at)
    lui $at, %hi(audio_cue_work + 0xd8)
    sh $v0, %lo(audio_cue_work + 0xd8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1f1)
    addu $at, $at, $a1
    lbu $v1, %lo(audio_loader_auxiliary + 0x1f1)($at)
    nop
    andi $v1, $v1, 0x7f
    andi $v1, $v1, 0xff
    lui $at, %hi(audio_cue_work)
    sh $v1, %lo(audio_cue_work)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1f1)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x1f1)($at)
    nop
    andi $v0, $v0, 0x80
    lui $at, %hi(audio_cue_work + 0xec)
    sh $v0, %lo(audio_cue_work + 0xec)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1f2)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x1f2)($at)
    lui $t4, %hi(audio_loader_header_pointers + 0x10)
    lw $t4, %lo(audio_loader_header_pointers + 0x10)($t4)
    srl $a2, $v0, 4
    lui $at, %hi(audio_cue_work + 0x8)
    sh $a2, %lo(audio_cue_work + 0x8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1f2)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x1f2)($at)
    sll $v1, $v1, 1
    andi $v0, $v0, 0xf
    lui $at, %hi(audio_cue_work + 0xe0)
    sh $v0, %lo(audio_cue_work + 0xe0)($at)
    sll $v0, $a2, 1
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v0, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $v1
    lhu $v1, %lo(audio_cue_lookup)($at)
    addiu $v0, $v0, 0x820
    addu $v1, $v1, $v0
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v1, %lo(audio_cue_work + 0x104)($at)
    andi $v1, $v1, 0xffff
    addu $v1, $v1, $t4
    lbu $t3, 6($v1)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $t3, %lo(audio_cue_work + 0xc)($at)
    lbu $t2, 5($v1)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $t2, %lo(audio_cue_work + 0x18)($at)
    lbu $v0, 3($v1)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v0, %lo(audio_cue_work + 0x24)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v0, %lo(audio_cue_work + 0x1c)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v0, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1f3)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x1f3)($at)
    lui $t0, %hi(audio_cue_lookup)
    addiu $t0, $t0, %lo(audio_cue_lookup)
    andi $v0, $v0, 0x1f
    andi $a3, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x10)
    sh $a3, %lo(audio_cue_work + 0x10)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x1f3)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x1f3)($at)
    lui $t1, %hi(audio_cue_lookup + 0x8)
    addiu $t1, $t1, %lo(audio_cue_lookup + 0x8)
    andi $v0, $v0, 0x60
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $v0, %lo(audio_cue_work + 0xdc)($at)
    sll $v0, $v0, 0x10
    beqz $v0, .Leffects_8016064c
    addiu $sp, $sp, -0x18
    sra $a0, $v0, 0x15
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $a0, %lo(audio_cue_work + 0xdc)($at)
    addu $v1, $a0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_80160454
    slti $v0, $v1, 3
    beqz $v0, .Leffects_8016039c
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_80160550
    nop
    j .Leffects_8016064c
    nop
.Leffects_8016039c:
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_8016064c
    nop
    lui $at, %hi(audio_loader_auxiliary + 0x1f1)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x1f1)($at)
    addu $v1, $a2, $a0
    lui $at, %hi(audio_cue_work + 0x80)
    sh $v1, %lo(audio_cue_work + 0x80)($at)
    sll $v1, $v1, 1
    addu $v1, $v1, $t1
    lhu $v1, 0($v1)
    lui $at, %hi(audio_cue_work + 0x84)
    sh $t3, %lo(audio_cue_work + 0x84)($at)
    lui $at, %hi(audio_cue_work + 0x90)
    sh $t2, %lo(audio_cue_work + 0x90)($at)
    andi $v0, $v0, 0x7f
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x78)
    sh $v0, %lo(audio_cue_work + 0x78)($at)
    sll $v0, $v0, 1
    addu $v0, $v0, $t0
    lhu $v0, 0($v0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $t4
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x94)
    sh $v1, %lo(audio_cue_work + 0x94)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x98)
    sh $v1, %lo(audio_cue_work + 0x98)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a3, 3
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x9c)
    sh $v1, %lo(audio_cue_work + 0x9c)($at)
    bnez $v0, .Leffects_80160454
    addiu $v0, $a3, -0x15
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
.Leffects_80160454:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x1f1)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x1f1)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x50)
    sh $a0, %lo(audio_cue_work + 0x50)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x58)
    sh $v0, %lo(audio_cue_work + 0x58)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0x10)
    lw $v1, %lo(audio_loader_header_pointers + 0x10)($v1)
    lui $at, %hi(audio_cue_work + 0x5c)
    sh $a1, %lo(audio_cue_work + 0x5c)($at)
    lui $at, %hi(audio_cue_work + 0x68)
    sh $a2, %lo(audio_cue_work + 0x68)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x6c)
    sh $v1, %lo(audio_cue_work + 0x6c)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x70)
    sh $v1, %lo(audio_cue_work + 0x70)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 2
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x74)
    sh $v1, %lo(audio_cue_work + 0x74)($at)
    bnez $v0, .Leffects_80160550
    addiu $v0, $a0, -0x16
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
.Leffects_80160550:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x1f1)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x1f1)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x28)
    sh $a0, %lo(audio_cue_work + 0x28)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x30)
    sh $v0, %lo(audio_cue_work + 0x30)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0x10)
    lw $v1, %lo(audio_loader_header_pointers + 0x10)($v1)
    lui $at, %hi(audio_cue_work + 0x34)
    sh $a1, %lo(audio_cue_work + 0x34)($at)
    lui $at, %hi(audio_cue_work + 0x40)
    sh $a2, %lo(audio_cue_work + 0x40)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x44)
    sh $v1, %lo(audio_cue_work + 0x44)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x48)
    sh $v1, %lo(audio_cue_work + 0x48)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x4c)
    sh $v1, %lo(audio_cue_work + 0x4c)($at)
    bnez $v0, .Leffects_8016064c
    addiu $v0, $a0, -0x17
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
.Leffects_8016064c:
    lui $v0, %hi(audio_cue_work + 0xd8)
    lh $v0, %lo(audio_cue_work + 0xd8)($v0)
    nop
    beqz $v0, .Leffects_80160810
    addu $v1, $v0, $zero
    andi $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    addu $v1, $v0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_80160720
    slti $v0, $v1, 3
    beqz $v0, .Leffects_80160694
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_801606a8
    nop
    j .Leffects_80160810
    nop
.Leffects_80160694:
    addiu $v0, $zero, 3
    beq $v1, $v0, .Leffects_80160798
    nop
    j .Leffects_80160810
    nop
.Leffects_801606a8:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x4)
    lw $v1, %lo(audio_loader_header_pointers + 0x4)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7f)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x7f)($at)
    j .Leffects_80160808
    nop
.Leffects_80160720:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x8)
    lw $v1, %lo(audio_loader_header_pointers + 0x8)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xfb)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0xfb)($at)
    j .Leffects_80160808
    nop
.Leffects_80160798:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0xc)
    lw $v1, %lo(audio_loader_header_pointers + 0xc)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x177)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x177)($at)
.Leffects_80160808:
    lui $at, %hi(audio_cue_work + 0x10)
    sh $v0, %lo(audio_cue_work + 0x10)($at)
.Leffects_80160810:
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cue_handler_4

.section .text
.balign 4
# exe/slus_004_22@0x8016081C..0x80160E38
.globl audio_cue_handler_5
.ent audio_cue_handler_5
audio_cue_handler_5:
    lui $v1, %hi(audio_cue_work + 0xfc)
    lhu $v1, %lo(audio_cue_work + 0xfc)($v1)
    addiu $v0, $zero, 5
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    sll $a1, $v1, 2
    lui $at, %hi(audio_loader_auxiliary + 0x26c)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x26c)($at)
    lui $at, %hi(audio_cue_work + 0xd8)
    sh $v0, %lo(audio_cue_work + 0xd8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x26d)
    addu $at, $at, $a1
    lbu $v1, %lo(audio_loader_auxiliary + 0x26d)($at)
    nop
    andi $v1, $v1, 0x7f
    andi $v1, $v1, 0xff
    lui $at, %hi(audio_cue_work)
    sh $v1, %lo(audio_cue_work)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x26d)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x26d)($at)
    nop
    andi $v0, $v0, 0x80
    lui $at, %hi(audio_cue_work + 0xec)
    sh $v0, %lo(audio_cue_work + 0xec)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x26e)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x26e)($at)
    lui $t4, %hi(audio_loader_header_pointers + 0x14)
    lw $t4, %lo(audio_loader_header_pointers + 0x14)($t4)
    srl $a2, $v0, 4
    lui $at, %hi(audio_cue_work + 0x8)
    sh $a2, %lo(audio_cue_work + 0x8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x26e)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x26e)($at)
    sll $v1, $v1, 1
    andi $v0, $v0, 0xf
    lui $at, %hi(audio_cue_work + 0xe0)
    sh $v0, %lo(audio_cue_work + 0xe0)($at)
    sll $v0, $a2, 1
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v0, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $v1
    lhu $v1, %lo(audio_cue_lookup)($at)
    addiu $v0, $v0, 0x820
    addu $v1, $v1, $v0
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v1, %lo(audio_cue_work + 0x104)($at)
    andi $v1, $v1, 0xffff
    addu $v1, $v1, $t4
    lbu $t3, 6($v1)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $t3, %lo(audio_cue_work + 0xc)($at)
    lbu $t2, 5($v1)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $t2, %lo(audio_cue_work + 0x18)($at)
    lbu $v0, 3($v1)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v0, %lo(audio_cue_work + 0x24)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v0, %lo(audio_cue_work + 0x1c)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v0, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x26f)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x26f)($at)
    lui $t0, %hi(audio_cue_lookup)
    addiu $t0, $t0, %lo(audio_cue_lookup)
    andi $v0, $v0, 0x1f
    andi $a3, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x10)
    sh $a3, %lo(audio_cue_work + 0x10)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x26f)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x26f)($at)
    lui $t1, %hi(audio_cue_lookup + 0x8)
    addiu $t1, $t1, %lo(audio_cue_lookup + 0x8)
    andi $v0, $v0, 0x60
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $v0, %lo(audio_cue_work + 0xdc)($at)
    sll $v0, $v0, 0x10
    beqz $v0, .Leffects_80160c68
    addiu $sp, $sp, -0x18
    sra $a0, $v0, 0x15
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $a0, %lo(audio_cue_work + 0xdc)($at)
    addu $v1, $a0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_80160a70
    slti $v0, $v1, 3
    beqz $v0, .Leffects_801609b8
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_80160b6c
    nop
    j .Leffects_80160c68
    nop
.Leffects_801609b8:
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_80160c68
    nop
    lui $at, %hi(audio_loader_auxiliary + 0x26d)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x26d)($at)
    addu $v1, $a2, $a0
    lui $at, %hi(audio_cue_work + 0x80)
    sh $v1, %lo(audio_cue_work + 0x80)($at)
    sll $v1, $v1, 1
    addu $v1, $v1, $t1
    lhu $v1, 0($v1)
    lui $at, %hi(audio_cue_work + 0x84)
    sh $t3, %lo(audio_cue_work + 0x84)($at)
    lui $at, %hi(audio_cue_work + 0x90)
    sh $t2, %lo(audio_cue_work + 0x90)($at)
    andi $v0, $v0, 0x7f
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x78)
    sh $v0, %lo(audio_cue_work + 0x78)($at)
    sll $v0, $v0, 1
    addu $v0, $v0, $t0
    lhu $v0, 0($v0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $t4
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x94)
    sh $v1, %lo(audio_cue_work + 0x94)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x98)
    sh $v1, %lo(audio_cue_work + 0x98)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a3, 3
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x9c)
    sh $v1, %lo(audio_cue_work + 0x9c)($at)
    bnez $v0, .Leffects_80160a70
    addiu $v0, $a3, -0x15
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
.Leffects_80160a70:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x26d)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x26d)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x50)
    sh $a0, %lo(audio_cue_work + 0x50)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x58)
    sh $v0, %lo(audio_cue_work + 0x58)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0x14)
    lw $v1, %lo(audio_loader_header_pointers + 0x14)($v1)
    lui $at, %hi(audio_cue_work + 0x5c)
    sh $a1, %lo(audio_cue_work + 0x5c)($at)
    lui $at, %hi(audio_cue_work + 0x68)
    sh $a2, %lo(audio_cue_work + 0x68)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x6c)
    sh $v1, %lo(audio_cue_work + 0x6c)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x70)
    sh $v1, %lo(audio_cue_work + 0x70)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 2
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x74)
    sh $v1, %lo(audio_cue_work + 0x74)($at)
    bnez $v0, .Leffects_80160b6c
    addiu $v0, $a0, -0x16
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
.Leffects_80160b6c:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x26d)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x26d)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x28)
    sh $a0, %lo(audio_cue_work + 0x28)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x30)
    sh $v0, %lo(audio_cue_work + 0x30)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0x14)
    lw $v1, %lo(audio_loader_header_pointers + 0x14)($v1)
    lui $at, %hi(audio_cue_work + 0x34)
    sh $a1, %lo(audio_cue_work + 0x34)($at)
    lui $at, %hi(audio_cue_work + 0x40)
    sh $a2, %lo(audio_cue_work + 0x40)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x44)
    sh $v1, %lo(audio_cue_work + 0x44)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x48)
    sh $v1, %lo(audio_cue_work + 0x48)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x4c)
    sh $v1, %lo(audio_cue_work + 0x4c)($at)
    bnez $v0, .Leffects_80160c68
    addiu $v0, $a0, -0x17
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
.Leffects_80160c68:
    lui $v0, %hi(audio_cue_work + 0xd8)
    lh $v0, %lo(audio_cue_work + 0xd8)($v0)
    nop
    beqz $v0, .Leffects_80160e2c
    addu $v1, $v0, $zero
    andi $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    addu $v1, $v0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_80160d3c
    slti $v0, $v1, 3
    beqz $v0, .Leffects_80160cb0
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_80160cc4
    nop
    j .Leffects_80160e2c
    nop
.Leffects_80160cb0:
    addiu $v0, $zero, 3
    beq $v1, $v0, .Leffects_80160db4
    nop
    j .Leffects_80160e2c
    nop
.Leffects_80160cc4:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x4)
    lw $v1, %lo(audio_loader_header_pointers + 0x4)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7f)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x7f)($at)
    j .Leffects_80160e24
    nop
.Leffects_80160d3c:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x8)
    lw $v1, %lo(audio_loader_header_pointers + 0x8)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xfb)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0xfb)($at)
    j .Leffects_80160e24
    nop
.Leffects_80160db4:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0xc)
    lw $v1, %lo(audio_loader_header_pointers + 0xc)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x177)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x177)($at)
.Leffects_80160e24:
    lui $at, %hi(audio_cue_work + 0x10)
    sh $v0, %lo(audio_cue_work + 0x10)($at)
.Leffects_80160e2c:
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cue_handler_5

.section .text
.balign 4
# exe/slus_004_22@0x80160E38..0x80161454
.globl audio_cue_handler_6
.ent audio_cue_handler_6
audio_cue_handler_6:
    lui $v1, %hi(audio_cue_work + 0xfc)
    lhu $v1, %lo(audio_cue_work + 0xfc)($v1)
    addiu $v0, $zero, 6
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    sll $a1, $v1, 2
    lui $at, %hi(audio_loader_auxiliary + 0x2e8)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x2e8)($at)
    lui $at, %hi(audio_cue_work + 0xd8)
    sh $v0, %lo(audio_cue_work + 0xd8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x2e9)
    addu $at, $at, $a1
    lbu $v1, %lo(audio_loader_auxiliary + 0x2e9)($at)
    nop
    andi $v1, $v1, 0x7f
    andi $v1, $v1, 0xff
    lui $at, %hi(audio_cue_work)
    sh $v1, %lo(audio_cue_work)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x2e9)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x2e9)($at)
    nop
    andi $v0, $v0, 0x80
    lui $at, %hi(audio_cue_work + 0xec)
    sh $v0, %lo(audio_cue_work + 0xec)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x2ea)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x2ea)($at)
    lui $t4, %hi(audio_loader_header_pointers + 0x18)
    lw $t4, %lo(audio_loader_header_pointers + 0x18)($t4)
    srl $a2, $v0, 4
    lui $at, %hi(audio_cue_work + 0x8)
    sh $a2, %lo(audio_cue_work + 0x8)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x2ea)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x2ea)($at)
    sll $v1, $v1, 1
    andi $v0, $v0, 0xf
    lui $at, %hi(audio_cue_work + 0xe0)
    sh $v0, %lo(audio_cue_work + 0xe0)($at)
    sll $v0, $a2, 1
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v0, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $v1
    lhu $v1, %lo(audio_cue_lookup)($at)
    addiu $v0, $v0, 0x820
    addu $v1, $v1, $v0
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v1, %lo(audio_cue_work + 0x104)($at)
    andi $v1, $v1, 0xffff
    addu $v1, $v1, $t4
    lbu $t3, 6($v1)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $t3, %lo(audio_cue_work + 0xc)($at)
    lbu $t2, 5($v1)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $t2, %lo(audio_cue_work + 0x18)($at)
    lbu $v0, 3($v1)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v0, %lo(audio_cue_work + 0x24)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v0, %lo(audio_cue_work + 0x1c)($at)
    lbu $v0, 2($v1)
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v0, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x2eb)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x2eb)($at)
    lui $t0, %hi(audio_cue_lookup)
    addiu $t0, $t0, %lo(audio_cue_lookup)
    andi $v0, $v0, 0x1f
    andi $a3, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x10)
    sh $a3, %lo(audio_cue_work + 0x10)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x2eb)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x2eb)($at)
    lui $t1, %hi(audio_cue_lookup + 0x8)
    addiu $t1, $t1, %lo(audio_cue_lookup + 0x8)
    andi $v0, $v0, 0x60
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $v0, %lo(audio_cue_work + 0xdc)($at)
    sll $v0, $v0, 0x10
    beqz $v0, .Leffects_80161284
    addiu $sp, $sp, -0x18
    sra $a0, $v0, 0x15
    lui $at, %hi(audio_cue_work + 0xdc)
    sh $a0, %lo(audio_cue_work + 0xdc)($at)
    addu $v1, $a0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_8016108c
    slti $v0, $v1, 3
    beqz $v0, .Leffects_80160fd4
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_80161188
    nop
    j .Leffects_80161284
    nop
.Leffects_80160fd4:
    addiu $v0, $zero, 3
    bne $v1, $v0, .Leffects_80161284
    nop
    lui $at, %hi(audio_loader_auxiliary + 0x2e9)
    addu $at, $at, $a1
    lbu $v0, %lo(audio_loader_auxiliary + 0x2e9)($at)
    addu $v1, $a2, $a0
    lui $at, %hi(audio_cue_work + 0x80)
    sh $v1, %lo(audio_cue_work + 0x80)($at)
    sll $v1, $v1, 1
    addu $v1, $v1, $t1
    lhu $v1, 0($v1)
    lui $at, %hi(audio_cue_work + 0x84)
    sh $t3, %lo(audio_cue_work + 0x84)($at)
    lui $at, %hi(audio_cue_work + 0x90)
    sh $t2, %lo(audio_cue_work + 0x90)($at)
    andi $v0, $v0, 0x7f
    andi $v0, $v0, 0xff
    lui $at, %hi(audio_cue_work + 0x78)
    sh $v0, %lo(audio_cue_work + 0x78)($at)
    sll $v0, $v0, 1
    addu $v0, $v0, $t0
    lhu $v0, 0($v0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $t4
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x94)
    sh $v1, %lo(audio_cue_work + 0x94)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x98)
    sh $v1, %lo(audio_cue_work + 0x98)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a3, 3
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x9c)
    sh $v1, %lo(audio_cue_work + 0x9c)($at)
    bnez $v0, .Leffects_8016108c
    addiu $v0, $a3, -0x15
    lui $at, %hi(audio_cue_work + 0x88)
    sh $v0, %lo(audio_cue_work + 0x88)($at)
.Leffects_8016108c:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x2e9)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x2e9)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x50)
    sh $a0, %lo(audio_cue_work + 0x50)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x58)
    sh $v0, %lo(audio_cue_work + 0x58)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0x18)
    lw $v1, %lo(audio_loader_header_pointers + 0x18)($v1)
    lui $at, %hi(audio_cue_work + 0x5c)
    sh $a1, %lo(audio_cue_work + 0x5c)($at)
    lui $at, %hi(audio_cue_work + 0x68)
    sh $a2, %lo(audio_cue_work + 0x68)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x6c)
    sh $v1, %lo(audio_cue_work + 0x6c)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x70)
    sh $v1, %lo(audio_cue_work + 0x70)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 2
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x74)
    sh $v1, %lo(audio_cue_work + 0x74)($at)
    bnez $v0, .Leffects_80161188
    addiu $v0, $a0, -0x16
    lui $at, %hi(audio_cue_work + 0x60)
    sh $v0, %lo(audio_cue_work + 0x60)($at)
.Leffects_80161188:
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    lui $v1, %hi(audio_cue_work + 0xdc)
    lhu $v1, %lo(audio_cue_work + 0xdc)($v1)
    lui $a1, %hi(audio_cue_work + 0xc)
    lhu $a1, %lo(audio_cue_work + 0xc)($a1)
    sll $v0, $v0, 2
    lui $at, %hi(audio_loader_auxiliary + 0x2e9)
    addu $at, $at, $v0
    lbu $a0, %lo(audio_loader_auxiliary + 0x2e9)($at)
    lui $v0, %hi(audio_cue_work + 0x8)
    lhu $v0, %lo(audio_cue_work + 0x8)($v0)
    lui $a2, %hi(audio_cue_work + 0x18)
    lhu $a2, %lo(audio_cue_work + 0x18)($a2)
    addu $v0, $v0, $v1
    andi $a0, $a0, 0x7f
    andi $a0, $a0, 0xff
    lui $at, %hi(audio_cue_work + 0x28)
    sh $a0, %lo(audio_cue_work + 0x28)($at)
    sll $a0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x30)
    sh $v0, %lo(audio_cue_work + 0x30)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0xf
    lui $at, %hi(audio_cue_lookup + 0x8)
    addu $at, $at, $v0
    lhu $v1, %lo(audio_cue_lookup + 0x8)($at)
    lui $at, %hi(audio_cue_lookup)
    addu $at, $at, $a0
    lhu $v0, %lo(audio_cue_lookup)($at)
    lui $a0, %hi(audio_cue_work + 0x10)
    lhu $a0, %lo(audio_cue_work + 0x10)($a0)
    addiu $v1, $v1, 0x820
    addu $v0, $v0, $v1
    lui $v1, %hi(audio_loader_header_pointers + 0x18)
    lw $v1, %lo(audio_loader_header_pointers + 0x18)($v1)
    lui $at, %hi(audio_cue_work + 0x34)
    sh $a1, %lo(audio_cue_work + 0x34)($at)
    lui $at, %hi(audio_cue_work + 0x40)
    sh $a2, %lo(audio_cue_work + 0x40)($at)
    lui $at, %hi(audio_cue_work + 0x104)
    sh $v0, %lo(audio_cue_work + 0x104)($at)
    andi $v0, $v0, 0xffff
    addu $v0, $v0, $v1
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x44)
    sh $v1, %lo(audio_cue_work + 0x44)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x48)
    sh $v1, %lo(audio_cue_work + 0x48)($at)
    lbu $v1, 3($v0)
    addiu $v0, $a0, 1
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x18
    lui $at, %hi(audio_cue_work + 0x4c)
    sh $v1, %lo(audio_cue_work + 0x4c)($at)
    bnez $v0, .Leffects_80161284
    addiu $v0, $a0, -0x17
    lui $at, %hi(audio_cue_work + 0x38)
    sh $v0, %lo(audio_cue_work + 0x38)($at)
.Leffects_80161284:
    lui $v0, %hi(audio_cue_work + 0xd8)
    lh $v0, %lo(audio_cue_work + 0xd8)($v0)
    nop
    beqz $v0, .Leffects_80161448
    addu $v1, $v0, $zero
    andi $v0, $v1, 7
    lui $at, %hi(audio_cue_work + 0x4)
    sh $v0, %lo(audio_cue_work + 0x4)($at)
    addu $v1, $v0, $zero
    addiu $v0, $zero, 2
    beq $v1, $v0, .Leffects_80161358
    slti $v0, $v1, 3
    beqz $v0, .Leffects_801612cc
    addiu $v0, $zero, 1
    beq $v1, $v0, .Leffects_801612e0
    nop
    j .Leffects_80161448
    nop
.Leffects_801612cc:
    addiu $v0, $zero, 3
    beq $v1, $v0, .Leffects_801613d0
    nop
    j .Leffects_80161448
    nop
.Leffects_801612e0:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x4)
    lw $v1, %lo(audio_loader_header_pointers + 0x4)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x7f)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x7f)($at)
    j .Leffects_80161440
    nop
.Leffects_80161358:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0x8)
    lw $v1, %lo(audio_loader_header_pointers + 0x8)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0xfb)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0xfb)($at)
    j .Leffects_80161440
    nop
.Leffects_801613d0:
    lui $v0, %hi(audio_cue_work + 0x104)
    lhu $v0, %lo(audio_cue_work + 0x104)($v0)
    lui $v1, %hi(audio_loader_header_pointers + 0xc)
    lw $v1, %lo(audio_loader_header_pointers + 0xc)($v1)
    nop
    addu $v0, $v0, $v1
    lbu $v1, 6($v0)
    lui $at, %hi(audio_cue_work + 0xc)
    sh $v1, %lo(audio_cue_work + 0xc)($at)
    lbu $v1, 5($v0)
    lui $at, %hi(audio_cue_work + 0x18)
    sh $v1, %lo(audio_cue_work + 0x18)($at)
    lbu $v1, 3($v0)
    lui $at, %hi(audio_cue_work + 0x24)
    sh $v1, %lo(audio_cue_work + 0x24)($at)
    lbu $v1, 2($v0)
    lui $at, %hi(audio_cue_work + 0x1c)
    sh $v1, %lo(audio_cue_work + 0x1c)($at)
    lbu $v1, 2($v0)
    lui $v0, %hi(audio_cue_work + 0xfc)
    lhu $v0, %lo(audio_cue_work + 0xfc)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cue_work + 0x20)
    sh $v1, %lo(audio_cue_work + 0x20)($at)
    lui $at, %hi(audio_loader_auxiliary + 0x177)
    addu $at, $at, $v0
    lbu $v0, %lo(audio_loader_auxiliary + 0x177)($at)
.Leffects_80161440:
    lui $at, %hi(audio_cue_work + 0x10)
    sh $v0, %lo(audio_cue_work + 0x10)($at)
.Leffects_80161448:
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cue_handler_6

.section .text
.balign 4
# exe/slus_004_22@0x8016F8F8..0x8016F964
.globl audio_effects_set_voice_volume
.ent audio_effects_set_voice_volume
audio_effects_set_voice_volume:
    addiu $sp, $sp, -8
    andi $v0, $a0, 0xffff
    sltiu $v0, $v0, 0x18
    bnez $v0, .Leffects_8016f914
    addu $a3, $a1, $zero
    j .Leffects_8016f958
    addiu $v0, $zero, -1
.Leffects_8016f914:
    sll $v1, $a0, 0x10
    sra $v1, $v1, 0x10
    sll $a1, $v1, 4
    lui $at, %hi(audio_sequence_voice_flags + 0x12)
    addu $at, $at, $a1
    sh $a2, %lo(audio_sequence_voice_flags + 0x12)($at)
    lui $at, %hi(audio_sequence_voice_globals + 0x598)
    addu $at, $at, $v1
    lbu $a0, %lo(audio_sequence_voice_globals + 0x598)($at)
    addu $v0, $zero, $zero
    lui $at, %hi(audio_sequence_voice_flags + 0x10)
    addu $at, $at, $a1
    sh $a3, %lo(audio_sequence_voice_flags + 0x10)($at)
    ori $a0, $a0, 3
    lui $at, %hi(audio_sequence_voice_globals + 0x598)
    addu $at, $at, $v1
    sb $a0, %lo(audio_sequence_voice_globals + 0x598)($at)
.Leffects_8016f958:
    addiu $sp, $sp, 8
    jr $ra
    nop
.end audio_effects_set_voice_volume

.section .data
.balign 4
# exe/slus_004_22@0x801821E0..0x801822E0; overlapping writable lookups.
.globl audio_cue_lookup
audio_cue_lookup:
    .byte 0x00, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x40, 0x00, 0x60, 0x00
    .byte 0x80, 0x00, 0xa0, 0x00, 0xc0, 0x00, 0xe0, 0x00, 0x00, 0x01, 0x20, 0x01, 0x40, 0x01, 0x60, 0x01
    .byte 0x80, 0x01, 0xa0, 0x01, 0xc0, 0x01, 0xe0, 0x01, 0x00, 0x02, 0x20, 0x02, 0x40, 0x02, 0x60, 0x02
    .byte 0x80, 0x02, 0xa0, 0x02, 0xc0, 0x02, 0xe0, 0x02, 0x00, 0x03, 0x20, 0x03, 0x40, 0x03, 0x60, 0x03
    .byte 0x80, 0x03, 0xa0, 0x03, 0xc0, 0x03, 0xe0, 0x03, 0x00, 0x04, 0x20, 0x04, 0x40, 0x04, 0x60, 0x04
    .byte 0x80, 0x04, 0xa0, 0x04, 0xc0, 0x04, 0xe0, 0x04, 0x00, 0x05, 0x20, 0x05, 0x40, 0x05, 0x60, 0x05
    .byte 0x80, 0x05, 0xa0, 0x05, 0xc0, 0x05, 0xe0, 0x05, 0x00, 0x06, 0x20, 0x06, 0x40, 0x06, 0x60, 0x06
    .byte 0x80, 0x06, 0xa0, 0x06, 0xc0, 0x06, 0xe0, 0x06, 0x00, 0x07, 0x20, 0x07, 0x40, 0x07, 0x60, 0x07
    .byte 0x80, 0x07, 0xa0, 0x07, 0xc0, 0x07, 0xe0, 0x07, 0x00, 0x08, 0x20, 0x08, 0x40, 0x08, 0x60, 0x08
    .byte 0x80, 0x08, 0xa0, 0x08, 0xc0, 0x08, 0xe0, 0x08, 0x00, 0x09, 0x20, 0x09, 0x40, 0x09, 0x60, 0x09
    .byte 0x80, 0x09, 0xa0, 0x09, 0xc0, 0x09, 0xe0, 0x09, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3e, 0xff, 0x3d
    .byte 0xff, 0x3c, 0xff, 0x3b, 0xff, 0x3a, 0xff, 0x39, 0xff, 0x38, 0xff, 0x37, 0xff, 0x36, 0xff, 0x37
    .byte 0xff, 0x34, 0xff, 0x33, 0xff, 0x32, 0xff, 0x31, 0xff, 0x30, 0xff, 0x2f, 0xff, 0x2e, 0xff, 0x2d
    .byte 0xff, 0x2c, 0xff, 0x2b, 0xff, 0x2a, 0xff, 0x29, 0xff, 0x28, 0xff, 0x27, 0xff, 0x26, 0xff, 0x27
    .byte 0xff, 0x24, 0xff, 0x23, 0xff, 0x22, 0xff, 0x21, 0xff, 0x20, 0xff, 0x1f, 0xff, 0x1e, 0xff, 0x1d
    .byte 0xff, 0x1c, 0xff, 0x1b, 0xff, 0x1a, 0xff, 0x19, 0xff, 0x18, 0xff, 0x17, 0xff, 0x16, 0xff, 0x17
