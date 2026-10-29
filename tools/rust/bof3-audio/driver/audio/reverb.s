# SPDX-License-Identifier: MIT
# Owned SPU reverb modes, attributes, registers and work-area clearing.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Retain complete functions, live caller data and device/event side effects.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x8016E8A4..0x8016ED9C
.globl audio_spu_set_reverb_attributes
.ent audio_spu_set_reverb_attributes
audio_spu_set_reverb_attributes:
    addiu $sp, $sp, -0x90
    sw $s2, 0x70($sp)
    addu $s2, $a0, $zero
    sw $s7, 0x84($sp)
    addu $s7, $zero, $zero
    sw $s4, 0x78($sp)
    addu $s4, $zero, $zero
    sw $s6, 0x80($sp)
    addu $s6, $zero, $zero
    sw $ra, 0x8c($sp)
    sw $fp, 0x88($sp)
    sw $s5, 0x7c($sp)
    sw $s3, 0x74($sp)
    sw $s1, 0x6c($sp)
    sw $s0, 0x68($sp)
    sw $zero, 0x58($sp)
    lw $s3, 0($s2)
    addu $fp, $zero, $zero
    sltiu $s5, $s3, 1
    bnez $s5, .Lreverb_8016e904
    sw $zero, 0x10($sp)
    andi $v0, $s3, 1
    beqz $v0, .Lreverb_8016ea24
    nop
.Lreverb_8016e904:
    lw $s0, 4($s2)
    nop
    andi $v0, $s0, 0x100
    beqz $v0, .Lreverb_8016e924
    addiu $v0, $zero, -0x101
    and $s0, $s0, $v0
    addiu $t0, $zero, 1
    sw $t0, 0x58($sp)
.Lreverb_8016e924:
    sltiu $v0, $s0, 0xa
    beqz $v0, .Lreverb_8016e954
    sll $v0, $s0, 2
    lui $at, %hi(audio_spu_initial_reverb_start)
    addu $at, $at, $v0
    lw $a0, %lo(audio_spu_initial_reverb_start)($at)
    lui $s1, %hi(audio_spu_initial_reverb_start)
    addiu $s1, $s1, %lo(audio_spu_initial_reverb_start)
    jal audio_spu_reverb_allocation_conflict
    nop
    beqz $v0, .Lreverb_8016e95c
    addiu $s4, $zero, 1
.Lreverb_8016e954:
    j .Lreverb_8016ed68
    addiu $v0, $zero, -1
.Lreverb_8016e95c:
    addiu $a2, $sp, 0x10
    addiu $a1, $zero, 0x43
    lui $at, %hi(audio_spu_state_3b30)
    sw $s0, %lo(audio_spu_state_3b30)($at)
    lui $v1, %hi(audio_spu_state_3b30)
    lw $v1, %lo(audio_spu_state_3b30)($v1)
    addiu $a3, $zero, -1
    sll $a0, $v1, 2
    addu $a0, $a0, $s1
    sll $v0, $v1, 4
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $v1, %hi(audio_spu_reverb_presets)
    addiu $v1, $v1, %lo(audio_spu_reverb_presets)
    lw $a0, 0($a0)
    addu $v1, $v0, $v1
    lui $at, %hi(audio_spu_state_3b28)
    sw $a0, %lo(audio_spu_state_3b28)($at)
.Lreverb_8016e9a4:
    lbu $v0, 0($v1)
    addiu $v1, $v1, 1
    addiu $a1, $a1, -1
    sb $v0, 0($a2)
    bne $a1, $a3, .Lreverb_8016e9a4
    addiu $a2, $a2, 1
    lui $v1, %hi(audio_spu_state_3b30)
    lw $v1, %lo(audio_spu_state_3b30)($v1)
    addiu $v0, $zero, 7
    beq $v1, $v0, .Lreverb_8016e9e0
    addiu $v0, $zero, 8
    beq $v1, $v0, .Lreverb_8016e9fc
    addiu $v0, $zero, 0x7f
    j .Lreverb_8016ea14
    nop
.Lreverb_8016e9e0:
    addiu $v0, $zero, 0x7f
    lui $at, %hi(audio_spu_state_3b3c)
    sw $v0, %lo(audio_spu_state_3b3c)($at)
    lui $at, %hi(audio_spu_state_3b38)
    sw $v0, %lo(audio_spu_state_3b38)($at)
    j .Lreverb_8016ea24
    nop
.Lreverb_8016e9fc:
    lui $at, %hi(audio_spu_state_3b3c)
    sw $zero, %lo(audio_spu_state_3b3c)($at)
    lui $at, %hi(audio_spu_state_3b38)
    sw $v0, %lo(audio_spu_state_3b38)($at)
    j .Lreverb_8016ea24
    nop
.Lreverb_8016ea14:
    lui $at, %hi(audio_spu_state_3b3c)
    sw $zero, %lo(audio_spu_state_3b3c)($at)
    lui $at, %hi(audio_spu_state_3b38)
    sw $zero, %lo(audio_spu_state_3b38)($at)
.Lreverb_8016ea24:
    bnez $s5, .Lreverb_8016ea34
    andi $v0, $s3, 8
    beqz $v0, .Lreverb_8016eb44
    nop
.Lreverb_8016ea34:
    lui $v1, %hi(audio_spu_state_3b30)
    lw $v1, %lo(audio_spu_state_3b30)($v1)
    nop
    slti $v0, $v1, 9
    beqz $v0, .Lreverb_8016eb44
    slti $v0, $v1, 7
    bnez $v0, .Lreverb_8016eb44
    nop
    bnez $s4, .Lreverb_8016eaac
    addiu $s6, $zero, 1
    addiu $a1, $sp, 0x10
    addiu $a0, $zero, 0x43
    lui $v0, %hi(audio_spu_state_3b30)
    lw $v0, %lo(audio_spu_state_3b30)($v0)
    addiu $a2, $zero, -1
    sll $v1, $v0, 4
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    lui $v0, %hi(audio_spu_reverb_presets)
    addiu $v0, $v0, %lo(audio_spu_reverb_presets)
    addu $v1, $v1, $v0
.Lreverb_8016ea88:
    lbu $v0, 0($v1)
    addiu $v1, $v1, 1
    addiu $a0, $a0, -1
    sb $v0, 0($a1)
    bne $a0, $a2, .Lreverb_8016ea88
    addiu $a1, $a1, 1
    lui $v0, 0xc01
    ori $v0, $v0, 0x1c00
    sw $v0, 0x10($sp)
.Lreverb_8016eaac:
    lui $a0, 0x8102
    lw $v0, 0xc($s2)
    ori $a0, $a0, 0x409
    sll $v1, $v0, 0xd
    mult $v1, $a0
    mfhi $a3
    sll $a1, $v0, 0xc
    nop
    mult $a1, $a0
    lui $at, %hi(audio_spu_state_3b38)
    sw $v0, %lo(audio_spu_state_3b38)($at)
    addu $v0, $a3, $v1
    sra $v0, $v0, 6
    sra $v1, $v1, 0x1f
    subu $v0, $v0, $v1
    lhu $a0, 0x14($sp)
    lhu $v1, 0x36($sp)
    subu $v0, $v0, $a0
    sh $v0, 0x28($sp)
    lhu $v0, 0x16($sp)
    mfhi $a2
    addu $a0, $a2, $a1
    sra $a0, $a0, 6
    sra $a1, $a1, 0x1f
    subu $a0, $a0, $a1
    subu $v0, $a0, $v0
    sh $v0, 0x2a($sp)
    lhu $v0, 0x2e($sp)
    addu $v1, $v1, $a0
    sh $v1, 0x34($sp)
    lhu $v1, 0x4e($sp)
    addu $v0, $v0, $a0
    sh $v0, 0x2c($sp)
    lhu $v0, 0x4c($sp)
    addu $v1, $v1, $a0
    sh $v1, 0x4a($sp)
    addu $v0, $v0, $a0
    sh $v0, 0x48($sp)
.Lreverb_8016eb44:
    bnez $s5, .Lreverb_8016eb54
    andi $v0, $s3, 0x10
    beqz $v0, .Lreverb_8016ec18
    nop
.Lreverb_8016eb54:
    lui $v1, %hi(audio_spu_state_3b30)
    lw $v1, %lo(audio_spu_state_3b30)($v1)
    nop
    slti $v0, $v1, 9
    beqz $v0, .Lreverb_8016ec18
    slti $v0, $v1, 7
    bnez $v0, .Lreverb_8016ec18
    nop
    bnez $s4, .Lreverb_8016ebdc
    addiu $fp, $zero, 1
    bnez $s6, .Lreverb_8016ebcc
    addiu $a1, $sp, 0x10
    addiu $a0, $zero, 0x43
    lui $v0, %hi(audio_spu_state_3b30)
    lw $v0, %lo(audio_spu_state_3b30)($v0)
    addiu $a2, $zero, -1
    sll $v1, $v0, 4
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    lui $v0, %hi(audio_spu_reverb_presets)
    addiu $v0, $v0, %lo(audio_spu_reverb_presets)
    addu $v1, $v1, $v0
.Lreverb_8016ebac:
    lbu $v0, 0($v1)
    addiu $v1, $v1, 1
    addiu $a0, $a0, -1
    sb $v0, 0($a1)
    bne $a0, $a2, .Lreverb_8016ebac
    addiu $a1, $a1, 1
    j .Lreverb_8016ebd8
    addiu $v0, $zero, 0x80
.Lreverb_8016ebcc:
    lw $v0, 0x10($sp)
    nop
    ori $v0, $v0, 0x80
.Lreverb_8016ebd8:
    sw $v0, 0x10($sp)
.Lreverb_8016ebdc:
    lui $a0, 0x8102
    lw $v1, 0x10($s2)
    ori $a0, $a0, 0x409
    sll $v0, $v1, 7
    addu $v0, $v0, $v1
    sll $v0, $v0, 8
    mult $v0, $a0
    lui $at, %hi(audio_spu_state_3b3c)
    sw $v1, %lo(audio_spu_state_3b3c)($at)
    mfhi $t0
    addu $v1, $t0, $v0
    sra $v1, $v1, 6
    sra $v0, $v0, 0x1f
    subu $v1, $v1, $v0
    sh $v1, 0x22($sp)
.Lreverb_8016ec18:
    beqz $s4, .Lreverb_8016ec5c
    nop
    lui $a0, %hi(audio_spu_registers_pointer)
    lw $a0, %lo(audio_spu_registers_pointer)($a0)
    nop
    lhu $v0, 0x1aa($a0)
    nop
    srl $v0, $v0, 7
    andi $s7, $v0, 1
    beqz $s7, .Lreverb_8016ecc4
    nop
    lhu $v1, 0x1aa($a0)
    nop
    andi $v1, $v1, 0xff7f
    sh $v1, 0x1aa($a0)
    j .Lreverb_8016ecc4
    nop
.Lreverb_8016ec5c:
    bnez $s5, .Lreverb_8016ec6c
    andi $v0, $s3, 2
    beqz $v0, .Lreverb_8016ec8c
    nop
.Lreverb_8016ec6c:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    lhu $v1, 8($s2)
    nop
    sh $v1, 0x184($v0)
    lhu $v0, 8($s2)
    lui $at, %hi(audio_spu_state_3b34)
    sh $v0, %lo(audio_spu_state_3b34)($at)
.Lreverb_8016ec8c:
    bnez $s5, .Lreverb_8016ec9c
    andi $v0, $s3, 4
    beqz $v0, .Lreverb_8016ece8
    nop
.Lreverb_8016ec9c:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    lhu $v1, 0xa($s2)
    nop
    sh $v1, 0x186($v0)
    lhu $v0, 0xa($s2)
    lui $at, %hi(audio_spu_state_3b36)
    sh $v0, %lo(audio_spu_state_3b36)($at)
    j .Lreverb_8016ece8
    nop
.Lreverb_8016ecc4:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    sh $zero, 0x184($v0)
    sh $zero, 0x186($v0)
    lui $at, %hi(audio_spu_state_3b34)
    sh $zero, %lo(audio_spu_state_3b34)($at)
    lui $at, %hi(audio_spu_state_3b36)
    sh $zero, %lo(audio_spu_state_3b36)($at)
.Lreverb_8016ece8:
    bnez $s4, .Lreverb_8016ed00
    nop
    bnez $s6, .Lreverb_8016ed00
    nop
    beqz $fp, .Lreverb_8016ed08
    nop
.Lreverb_8016ed00:
    jal audio_spu_set_reverb_registers
    addiu $a0, $sp, 0x10
.Lreverb_8016ed08:
    lw $t0, 0x58($sp)
    nop
    beqz $t0, .Lreverb_8016ed28
    nop
    lui $a0, %hi(audio_spu_state_3b30)
    lw $a0, %lo(audio_spu_state_3b30)($a0)
    jal audio_spu_clear_reverb
    nop
.Lreverb_8016ed28:
    beqz $s4, .Lreverb_8016ed64
    addiu $a0, $zero, 0xd1
    lui $a1, %hi(audio_spu_state_3b28)
    lw $a1, %lo(audio_spu_state_3b28)($a1)
    jal audio_spu_write_register
    addu $a2, $zero, $zero
    beqz $s7, .Lreverb_8016ed68
    addu $v0, $zero, $zero
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    nop
    ori $v1, $v1, 0x80
    sh $v1, 0x1aa($v0)
.Lreverb_8016ed64:
    addu $v0, $zero, $zero
.Lreverb_8016ed68:
    lw $ra, 0x8c($sp)
    lw $fp, 0x88($sp)
    lw $s7, 0x84($sp)
    lw $s6, 0x80($sp)
    lw $s5, 0x7c($sp)
    lw $s4, 0x78($sp)
    lw $s3, 0x74($sp)
    lw $s2, 0x70($sp)
    lw $s1, 0x6c($sp)
    lw $s0, 0x68($sp)
    addiu $sp, $sp, 0x90
    jr $ra
    nop
.end audio_spu_set_reverb_attributes

# exe/slus_004_22@0x8016EE1C..0x8016EEA8
.globl audio_spu_reverb_allocation_conflict
.ent audio_spu_reverb_allocation_conflict
audio_spu_reverb_allocation_conflict:
    lui $v0, %hi(audio_spu_transfer_shift)
    lw $v0, %lo(audio_spu_transfer_shift)($v0)
    lui $v1, %hi(audio_spu_state_3f88)
    lw $v1, %lo(audio_spu_state_3f88)($v1)
    nop
    bnez $v1, .Lreverb_8016ee40
    sllv $a0, $a0, $v0
    j .Lreverb_8016eea0
    addu $v0, $zero, $zero
.Lreverb_8016ee40:
    lui $t0, 0x8000
    lui $a3, 0x4000
    lui $a2, 0xfff
    ori $a2, $a2, 0xffff
    addu $a1, $v1, $zero
.Lreverb_8016ee54:
    lw $v1, 0($a1)
    nop
    and $v0, $v1, $t0
    bnez $v0, .Lreverb_8016ee94
    and $v0, $v1, $a3
    bnez $v0, .Lreverb_8016ee9c
    and $v1, $v1, $a2
    sltu $v0, $v1, $a0
    beqz $v0, .Lreverb_8016eea0
    addiu $v0, $zero, 1
    lw $v0, 4($a1)
    nop
    addu $v0, $v1, $v0
    sltu $v0, $a0, $v0
    bnez $v0, .Lreverb_8016eea0
    addiu $v0, $zero, 1
.Lreverb_8016ee94:
    j .Lreverb_8016ee54
    addiu $a1, $a1, 8
.Lreverb_8016ee9c:
    addu $v0, $zero, $zero
.Lreverb_8016eea0:
    jr $ra
    nop
.end audio_spu_reverb_allocation_conflict

# exe/slus_004_22@0x8016EEA8..0x8016F378
.globl audio_spu_set_reverb_registers
.ent audio_spu_set_reverb_registers
audio_spu_set_reverb_registers:
    lw $a1, 0($a0)
    nop
    sltiu $a2, $a1, 1
    bnez $a2, .Lreverb_8016eec4
    andi $v0, $a1, 1
    beqz $v0, .Lreverb_8016eed8
    nop
.Lreverb_8016eec4:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 4($a0)
    nop
    sh $v0, 0x1c0($v1)
.Lreverb_8016eed8:
    bnez $a2, .Lreverb_8016eee8
    andi $v0, $a1, 2
    beqz $v0, .Lreverb_8016eefc
    nop
.Lreverb_8016eee8:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 6($a0)
    nop
    sh $v0, 0x1c2($v1)
.Lreverb_8016eefc:
    bnez $a2, .Lreverb_8016ef0c
    andi $v0, $a1, 4
    beqz $v0, .Lreverb_8016ef20
    nop
.Lreverb_8016ef0c:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 8($a0)
    nop
    sh $v0, 0x1c4($v1)
.Lreverb_8016ef20:
    bnez $a2, .Lreverb_8016ef30
    andi $v0, $a1, 8
    beqz $v0, .Lreverb_8016ef44
    nop
.Lreverb_8016ef30:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0xa($a0)
    nop
    sh $v0, 0x1c6($v1)
.Lreverb_8016ef44:
    bnez $a2, .Lreverb_8016ef54
    andi $v0, $a1, 0x10
    beqz $v0, .Lreverb_8016ef68
    nop
.Lreverb_8016ef54:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0xc($a0)
    nop
    sh $v0, 0x1c8($v1)
.Lreverb_8016ef68:
    bnez $a2, .Lreverb_8016ef78
    andi $v0, $a1, 0x20
    beqz $v0, .Lreverb_8016ef8c
    nop
.Lreverb_8016ef78:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0xe($a0)
    nop
    sh $v0, 0x1ca($v1)
.Lreverb_8016ef8c:
    bnez $a2, .Lreverb_8016ef9c
    andi $v0, $a1, 0x40
    beqz $v0, .Lreverb_8016efb0
    nop
.Lreverb_8016ef9c:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x10($a0)
    nop
    sh $v0, 0x1cc($v1)
.Lreverb_8016efb0:
    bnez $a2, .Lreverb_8016efc0
    andi $v0, $a1, 0x80
    beqz $v0, .Lreverb_8016efd4
    nop
.Lreverb_8016efc0:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x12($a0)
    nop
    sh $v0, 0x1ce($v1)
.Lreverb_8016efd4:
    bnez $a2, .Lreverb_8016efe4
    andi $v0, $a1, 0x100
    beqz $v0, .Lreverb_8016eff8
    nop
.Lreverb_8016efe4:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x14($a0)
    nop
    sh $v0, 0x1d0($v1)
.Lreverb_8016eff8:
    bnez $a2, .Lreverb_8016f008
    andi $v0, $a1, 0x200
    beqz $v0, .Lreverb_8016f01c
    nop
.Lreverb_8016f008:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x16($a0)
    nop
    sh $v0, 0x1d2($v1)
.Lreverb_8016f01c:
    bnez $a2, .Lreverb_8016f02c
    andi $v0, $a1, 0x400
    beqz $v0, .Lreverb_8016f040
    nop
.Lreverb_8016f02c:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x18($a0)
    nop
    sh $v0, 0x1d4($v1)
.Lreverb_8016f040:
    bnez $a2, .Lreverb_8016f050
    andi $v0, $a1, 0x800
    beqz $v0, .Lreverb_8016f064
    nop
.Lreverb_8016f050:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x1a($a0)
    nop
    sh $v0, 0x1d6($v1)
.Lreverb_8016f064:
    bnez $a2, .Lreverb_8016f074
    andi $v0, $a1, 0x1000
    beqz $v0, .Lreverb_8016f088
    nop
.Lreverb_8016f074:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x1c($a0)
    nop
    sh $v0, 0x1d8($v1)
.Lreverb_8016f088:
    bnez $a2, .Lreverb_8016f098
    andi $v0, $a1, 0x2000
    beqz $v0, .Lreverb_8016f0ac
    nop
.Lreverb_8016f098:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x1e($a0)
    nop
    sh $v0, 0x1da($v1)
.Lreverb_8016f0ac:
    bnez $a2, .Lreverb_8016f0bc
    andi $v0, $a1, 0x4000
    beqz $v0, .Lreverb_8016f0d0
    nop
.Lreverb_8016f0bc:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x20($a0)
    nop
    sh $v0, 0x1dc($v1)
.Lreverb_8016f0d0:
    bnez $a2, .Lreverb_8016f0e0
    andi $v0, $a1, 0x8000
    beqz $v0, .Lreverb_8016f0f4
    nop
.Lreverb_8016f0e0:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x22($a0)
    nop
    sh $v0, 0x1de($v1)
.Lreverb_8016f0f4:
    bnez $a2, .Lreverb_8016f108
    lui $v0, 1
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f11c
    nop
.Lreverb_8016f108:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x24($a0)
    nop
    sh $v0, 0x1e0($v1)
.Lreverb_8016f11c:
    bnez $a2, .Lreverb_8016f130
    lui $v0, 2
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f144
    nop
.Lreverb_8016f130:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x26($a0)
    nop
    sh $v0, 0x1e2($v1)
.Lreverb_8016f144:
    bnez $a2, .Lreverb_8016f158
    lui $v0, 4
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f16c
    nop
.Lreverb_8016f158:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x28($a0)
    nop
    sh $v0, 0x1e4($v1)
.Lreverb_8016f16c:
    bnez $a2, .Lreverb_8016f180
    lui $v0, 8
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f194
    nop
.Lreverb_8016f180:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x2a($a0)
    nop
    sh $v0, 0x1e6($v1)
.Lreverb_8016f194:
    bnez $a2, .Lreverb_8016f1a8
    lui $v0, 0x10
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f1bc
    nop
.Lreverb_8016f1a8:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x2c($a0)
    nop
    sh $v0, 0x1e8($v1)
.Lreverb_8016f1bc:
    bnez $a2, .Lreverb_8016f1d0
    lui $v0, 0x20
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f1e4
    nop
.Lreverb_8016f1d0:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x2e($a0)
    nop
    sh $v0, 0x1ea($v1)
.Lreverb_8016f1e4:
    bnez $a2, .Lreverb_8016f1f8
    lui $v0, 0x40
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f20c
    nop
.Lreverb_8016f1f8:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x30($a0)
    nop
    sh $v0, 0x1ec($v1)
.Lreverb_8016f20c:
    bnez $a2, .Lreverb_8016f220
    lui $v0, 0x80
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f234
    nop
.Lreverb_8016f220:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x32($a0)
    nop
    sh $v0, 0x1ee($v1)
.Lreverb_8016f234:
    bnez $a2, .Lreverb_8016f248
    lui $v0, 0x100
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f25c
    nop
.Lreverb_8016f248:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x34($a0)
    nop
    sh $v0, 0x1f0($v1)
.Lreverb_8016f25c:
    bnez $a2, .Lreverb_8016f270
    lui $v0, 0x200
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f284
    nop
.Lreverb_8016f270:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x36($a0)
    nop
    sh $v0, 0x1f2($v1)
.Lreverb_8016f284:
    bnez $a2, .Lreverb_8016f298
    lui $v0, 0x400
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f2ac
    nop
.Lreverb_8016f298:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x38($a0)
    nop
    sh $v0, 0x1f4($v1)
.Lreverb_8016f2ac:
    bnez $a2, .Lreverb_8016f2c0
    lui $v0, 0x800
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f2d4
    nop
.Lreverb_8016f2c0:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x3a($a0)
    nop
    sh $v0, 0x1f6($v1)
.Lreverb_8016f2d4:
    bnez $a2, .Lreverb_8016f2e8
    lui $v0, 0x1000
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f2fc
    nop
.Lreverb_8016f2e8:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x3c($a0)
    nop
    sh $v0, 0x1f8($v1)
.Lreverb_8016f2fc:
    bnez $a2, .Lreverb_8016f310
    lui $v0, 0x2000
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f324
    nop
.Lreverb_8016f310:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x3e($a0)
    nop
    sh $v0, 0x1fa($v1)
.Lreverb_8016f324:
    bnez $a2, .Lreverb_8016f338
    lui $v0, 0x4000
    and $v0, $a1, $v0
    beqz $v0, .Lreverb_8016f34c
    nop
.Lreverb_8016f338:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x40($a0)
    nop
    sh $v0, 0x1fc($v1)
.Lreverb_8016f34c:
    bnez $a2, .Lreverb_8016f35c
    nop
    bgez $a1, .Lreverb_8016f370
    nop
.Lreverb_8016f35c:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x42($a0)
    nop
    sh $v0, 0x1fe($v1)
.Lreverb_8016f370:
    jr $ra
    nop
.end audio_spu_set_reverb_registers

# exe/slus_004_22@0x8016F378..0x8016F514
.globl audio_spu_clear_reverb
.ent audio_spu_clear_reverb
audio_spu_clear_reverb:
    addiu $sp, $sp, -0x38
    sw $s0, 0x18($sp)
    addu $s0, $a0, $zero
    sw $s5, 0x2c($sp)
    sltiu $v0, $s0, 0xa
    sw $ra, 0x30($sp)
    sw $s4, 0x28($sp)
    sw $s3, 0x24($sp)
    sw $s2, 0x20($sp)
    sw $s1, 0x1c($sp)
    sw $zero, 0x10($sp)
    beqz $v0, .Lreverb_8016f3d0
    addu $s5, $zero, $zero
    lui $v1, %hi(audio_spu_initial_reverb_start)
    addiu $v1, $v1, %lo(audio_spu_initial_reverb_start)
    sll $v0, $s0, 2
    addu $s1, $v0, $v1
    lw $a0, 0($s1)
    jal audio_spu_reverb_allocation_conflict
    nop
    beqz $v0, .Lreverb_8016f3d8
    nop
.Lreverb_8016f3d0:
    j .Lreverb_8016f4ec
    addiu $v0, $zero, -1
.Lreverb_8016f3d8:
    bnez $s0, .Lreverb_8016f3fc
    lui $v0, 1
    lui $v0, %hi(audio_spu_transfer_shift)
    lw $v0, %lo(audio_spu_transfer_shift)($v0)
    addiu $v1, $zero, 0x10
    sllv $s1, $v1, $v0
    ori $v1, $zero, 0xfff0
    j .Lreverb_8016f414
    sllv $s2, $v1, $v0
.Lreverb_8016f3fc:
    lw $a0, 0($s1)
    lui $v1, %hi(audio_spu_transfer_shift)
    lw $v1, %lo(audio_spu_transfer_shift)($v1)
    subu $v0, $v0, $a0
    sllv $s1, $v0, $v1
    sllv $s2, $a0, $v1
.Lreverb_8016f414:
    lui $s4, %hi(audio_spu_state_3acc)
    lw $s4, %lo(audio_spu_state_3acc)($s4)
    addiu $v0, $zero, 1
    bne $s4, $v0, .Lreverb_8016f434
    nop
    lui $at, %hi(audio_spu_state_3acc)
    sw $zero, %lo(audio_spu_state_3acc)($at)
    addiu $s5, $zero, 1
.Lreverb_8016f434:
    lui $v0, %hi(audio_spu_transfer_callback)
    lw $v0, %lo(audio_spu_transfer_callback)($v0)
    nop
    beqz $v0, .Lreverb_8016f460
    addiu $s3, $zero, 1
    lui $v0, %hi(audio_spu_transfer_callback)
    lw $v0, %lo(audio_spu_transfer_callback)($v0)
    nop
    sw $v0, 0x10($sp)
    lui $at, %hi(audio_spu_transfer_callback)
    sw $zero, %lo(audio_spu_transfer_callback)($at)
.Lreverb_8016f460:
    sltiu $v0, $s1, 0x401
.Lreverb_8016f464:
    bnez $v0, .Lreverb_8016f474
    addu $s0, $s1, $zero
    j .Lreverb_8016f478
    addiu $s0, $zero, 0x400
.Lreverb_8016f474:
    addu $s3, $zero, $zero
.Lreverb_8016f478:
    addiu $a0, $zero, 2
    jal audio_spu_transfer
    addu $a1, $s2, $zero
    jal audio_spu_transfer
    addiu $a0, $zero, 1
    addiu $a0, $zero, 3
    lui $a1, %hi(audio_spu_reverb_zeroes)
    addiu $a1, $a1, %lo(audio_spu_reverb_zeroes)
    jal audio_spu_transfer
    addu $a2, $s0, $zero
    lui $a0, %hi(audio_spu_transfer_event)
    lw $a0, %lo(audio_spu_transfer_event)($a0)
    addiu $s1, $s1, -0x400
    jal audio_spu_service_wait_event
    addiu $s2, $s2, 0x400
    bnez $s3, .Lreverb_8016f464
    sltiu $v0, $s1, 0x401
    beqz $s5, .Lreverb_8016f4cc
    nop
    lui $at, %hi(audio_spu_state_3acc)
    sw $s4, %lo(audio_spu_state_3acc)($at)
.Lreverb_8016f4cc:
    lw $v0, 0x10($sp)
    nop
    beqz $v0, .Lreverb_8016f4ec
    addu $v0, $zero, $zero
    lw $v0, 0x10($sp)
    lui $at, %hi(audio_spu_transfer_callback)
    sw $v0, %lo(audio_spu_transfer_callback)($at)
    addu $v0, $zero, $zero
.Lreverb_8016f4ec:
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
.end audio_spu_clear_reverb

# exe/slus_004_22@0x8016F514..0x8016F5B8
.globl audio_sequence_set_reverb_mode
.ent audio_sequence_set_reverb_mode
audio_sequence_set_reverb_mode:
    addiu $sp, $sp, -0x18
    addu $a1, $zero, $zero
    addu $v1, $a0, $zero
    sll $v0, $a0, 0x10
    sw $ra, 0x14($sp)
    bgez $v0, .Lreverb_8016f538
    sw $s0, 0x10($sp)
    addiu $a1, $zero, 1
    negu $v1, $a0
.Lreverb_8016f538:
    andi $v0, $v1, 0xffff
    sltiu $v0, $v0, 0xa
    beqz $v0, .Lreverb_8016f5a0
    addiu $v0, $zero, 1
    lui $at, %hi(audio_sequence_reverb_attributes)
    sw $v0, %lo(audio_sequence_reverb_attributes)($at)
    beqz $a1, .Lreverb_8016f560
    ori $v0, $v1, 0x100
    j .Lreverb_8016f564
    sll $v0, $v0, 0x10
.Lreverb_8016f560:
    sll $v0, $v1, 0x10
.Lreverb_8016f564:
    sra $v0, $v0, 0x10
    lui $at, %hi(audio_sequence_reverb_attributes + 4)
    sw $v0, %lo(audio_sequence_reverb_attributes + 4)($at)
    sll $v0, $v1, 0x10
    sra $s0, $v0, 0x10
    bnez $s0, .Lreverb_8016f588
    nop
    jal audio_spu_set_reverb_enabled
    addu $a0, $zero, $zero
.Lreverb_8016f588:
    lui $a0, %hi(audio_sequence_reverb_attributes)
    addiu $a0, $a0, %lo(audio_sequence_reverb_attributes)
    jal audio_spu_set_reverb_attributes
    nop
    j .Lreverb_8016f5a4
    addu $v0, $s0, $zero
.Lreverb_8016f5a0:
    addiu $v0, $zero, -1
.Lreverb_8016f5a4:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_set_reverb_mode

# exe/slus_004_22@0x8016F5C8..0x8016F698
.globl audio_spu_set_reverb_enabled
.ent audio_spu_set_reverb_enabled
audio_spu_set_reverb_enabled:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    addu $s0, $a0, $zero
    beqz $s0, .Lreverb_8016f5f0
    sw $ra, 0x14($sp)
    addiu $v0, $zero, 1
    beq $s0, $v0, .Lreverb_8016f610
    nop
    j .Lreverb_8016f67c
    nop
.Lreverb_8016f5f0:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    lui $at, %hi(audio_spu_state_3b20)
    sw $zero, %lo(audio_spu_state_3b20)($at)
    j .Lreverb_8016f678
    andi $v1, $v1, 0xff7f
.Lreverb_8016f610:
    lui $v0, %hi(audio_spu_state_3b24)
    lw $v0, %lo(audio_spu_state_3b24)($v0)
    nop
    beq $v0, $s0, .Lreverb_8016f65c
    nop
    lui $a0, %hi(audio_spu_state_3b28)
    lw $a0, %lo(audio_spu_state_3b28)($a0)
    jal audio_spu_reverb_allocation_conflict
    nop
    beqz $v0, .Lreverb_8016f65c
    nop
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    lui $at, %hi(audio_spu_state_3b20)
    sw $zero, %lo(audio_spu_state_3b20)($at)
    j .Lreverb_8016f678
    andi $v1, $v1, 0xff7f
.Lreverb_8016f65c:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    lui $at, %hi(audio_spu_state_3b20)
    sw $s0, %lo(audio_spu_state_3b20)($at)
    ori $v1, $v1, 0x80
.Lreverb_8016f678:
    sh $v1, 0x1aa($v0)
.Lreverb_8016f67c:
    lui $v0, %hi(audio_spu_state_3b20)
    lw $v0, %lo(audio_spu_state_3b20)($v0)
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_spu_set_reverb_enabled

# exe/slus_004_22@0x8016F6B8..0x8016F6D8
.globl audio_sequence_enable_reverb
.ent audio_sequence_enable_reverb
audio_sequence_enable_reverb:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_spu_set_reverb_enabled
    addiu $a0, $zero, 1
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_enable_reverb

# exe/slus_004_22@0x8017ED5C, B0/0A WaitEvent.
.globl audio_spu_service_wait_event
audio_spu_service_wait_event:
    addiu $t2, $zero, 0xb0
    jr $t2
    addiu $t1, $zero, 0xa
    nop

.section .data
.balign 4

.section .bss
.balign 4
# exe/slus_004_22@0x80183B78, 1024 bytes.
.globl audio_spu_reverb_zeroes
audio_spu_reverb_zeroes:
    .space 1024
# exe/slus_004_22@0x8018E0D0, 20 bytes.
.globl audio_sequence_reverb_attributes
audio_sequence_reverb_attributes:
    .space 20
