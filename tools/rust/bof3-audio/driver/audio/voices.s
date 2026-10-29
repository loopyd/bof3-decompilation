# SPDX-License-Identifier: MIT
# Owned SPU voice attributes, pitch conversion and address writes.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# The preset and pitch tables retain their adjacency for signed pitch indices.
# Attribute storage and external SPU register-pointer bindings belong to callers.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x80168D28..0x80168DCC
.globl audio_spu_write_address
.ent audio_spu_write_address
audio_spu_write_address:
    lui $v0, %hi(audio_spu_state_3ad4)
    lw $v0, %lo(audio_spu_state_3ad4)($v0)
    nop
    beqz $v0, .Lvoice_80168d7c
    addu $a2, $a0, $zero
    lui $a0, %hi(audio_spu_state_3adc)
    lw $a0, %lo(audio_spu_state_3adc)($a0)
    nop
    divu $zero, $a1, $a0
    bnez $a0, .Lvoice_80168d58
    nop
    break 7
.Lvoice_80168d58:
    mfhi $v0
    nop
    beqz $v0, .Lvoice_80168d7c
    nop
    lui $v0, %hi(audio_spu_state_3ae0)
    lw $v0, %lo(audio_spu_state_3ae0)($v0)
    addu $a1, $a1, $a0
    nor $v0, $zero, $v0
    and $a1, $a1, $v0
.Lvoice_80168d7c:
    lui $v0, %hi(audio_spu_transfer_shift)
    lw $v0, %lo(audio_spu_transfer_shift)($v0)
    nop
    srlv $a3, $a1, $v0
    addiu $v0, $zero, -2
    beq $a2, $v0, .Lvoice_80168da8
    addiu $v0, $zero, -1
    bne $a2, $v0, .Lvoice_80168db0
    addu $v0, $a1, $zero
    j .Lvoice_80168dc4
    andi $v0, $a3, 0xffff
.Lvoice_80168da8:
    j .Lvoice_80168dc4
    addu $v0, $a1, $zero
.Lvoice_80168db0:
    lui $a0, %hi(audio_spu_registers_pointer)
    lw $a0, %lo(audio_spu_registers_pointer)($a0)
    sll $v1, $a2, 1
    addu $v1, $v1, $a0
    sh $a3, 0($v1)
.Lvoice_80168dc4:
    jr $ra
    nop
.end audio_spu_write_address

# exe/slus_004_22@0x801691D4..0x801691FC
.globl audio_spu_set_voice_attributes
.ent audio_spu_set_voice_attributes
audio_spu_set_voice_attributes:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    addu $a1, $zero, $zero
    addiu $a2, $zero, 0x17
    jal audio_spu_set_voice_range
    addu $a3, $zero, $zero
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_spu_set_voice_attributes

# exe/slus_004_22@0x801691FC..0x8016986C
.globl audio_spu_set_voice_range
.ent audio_spu_set_voice_range
audio_spu_set_voice_range:
    addiu $sp, $sp, -0x50
    sw $s0, 0x28($sp)
    addu $s0, $a0, $zero
    sw $s7, 0x44($sp)
    addu $s7, $a2, $zero
    sw $fp, 0x48($sp)
    addu $fp, $a3, $zero
    sw $s6, 0x40($sp)
    addu $s6, $zero, $zero
    sw $ra, 0x4c($sp)
    sw $s5, 0x3c($sp)
    sw $s4, 0x38($sp)
    sw $s3, 0x34($sp)
    sw $s2, 0x30($sp)
    sw $s1, 0x2c($sp)
    lw $s1, 4($s0)
    addu $s5, $zero, $zero
    bgez $a1, .Lvoice_8016924c
    sltiu $s2, $s1, 1
    addu $a1, $zero, $zero
.Lvoice_8016924c:
    slti $v0, $a1, 0x18
    beqz $v0, .Lvoice_80169274
    slti $v0, $s7, 0x18
    bnez $v0, .Lvoice_80169264
    nop
    addiu $s7, $zero, 0x17
.Lvoice_80169264:
    bltz $s7, .Lvoice_80169274
    slt $v0, $s7, $a1
    beqz $v0, .Lvoice_8016927c
    addiu $s7, $s7, 1
.Lvoice_80169274:
    j .Lvoice_80169838
    addiu $v0, $zero, -3
.Lvoice_8016927c:
    addu $s4, $a1, $zero
    slt $v0, $s4, $s7
    beqz $v0, .Lvoice_801697d8
    addiu $v0, $zero, 1
.Lvoice_8016928c:
    lw $v1, 0($s0)
    sllv $v0, $v0, $s4
    and $v1, $v1, $v0
    beqz $v1, .Lvoice_801697c8
    nop
    bnez $s2, .Lvoice_801692b4
    sll $s3, $s4, 3
    andi $v0, $s1, 0x10
    beqz $v0, .Lvoice_801692cc
    nop
.Lvoice_801692b4:
    sll $v0, $s4, 4
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $a0, 0x14($s0)
    addu $v0, $v0, $v1
    sh $a0, 4($v0)
.Lvoice_801692cc:
    bnez $s2, .Lvoice_801692dc
    andi $v0, $s1, 0x40
    beqz $v0, .Lvoice_801692f0
    nop
.Lvoice_801692dc:
    lhu $v1, 0x18($s0)
    sll $v0, $s4, 1
    lui $at, %hi(audio_spu_voice_control_shadow)
    addu $at, $at, $v0
    sh $v1, %lo(audio_spu_voice_control_shadow)($at)
.Lvoice_801692f0:
    bnez $s2, .Lvoice_80169304
    sll $v0, $s4, 1
    andi $v0, $s1, 0x20
    beqz $v0, .Lvoice_8016933c
    sll $v0, $s4, 1
.Lvoice_80169304:
    lui $at, %hi(audio_spu_voice_control_shadow)
    addu $at, $at, $v0
    lhu $a1, %lo(audio_spu_voice_control_shadow)($at)
    lhu $a3, 0x16($s0)
    srl $a0, $a1, 8
    andi $a1, $a1, 0xff
    srl $a2, $a3, 8
    jal audio_spu_note_to_pitch
    andi $a3, $a3, 0xff
    lui $a0, %hi(audio_spu_registers_pointer)
    lw $a0, %lo(audio_spu_registers_pointer)($a0)
    sll $v1, $s3, 1
    addu $v1, $v1, $a0
    sh $v0, 4($v1)
.Lvoice_8016933c:
    bnez $s2, .Lvoice_80169354
    sll $v0, $s3, 1
    lui $v0, 2
    and $v0, $s1, $v0
    beqz $v0, .Lvoice_80169368
    sll $v0, $s3, 1
.Lvoice_80169354:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $a0, 0x3a($s0)
    addu $v0, $v0, $v1
    sh $a0, 8($v0)
.Lvoice_80169368:
    bnez $s2, .Lvoice_80169380
    sll $v0, $s3, 1
    lui $v0, 4
    and $v0, $s1, $v0
    beqz $v0, .Lvoice_80169394
    sll $v0, $s3, 1
.Lvoice_80169380:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $a0, 0x3c($s0)
    addu $v0, $v0, $v1
    sh $a0, 0xa($v0)
.Lvoice_80169394:
    bnez $s2, .Lvoice_801693ac
    andi $v0, $s1, 1
    beqz $v0, .Lvoice_80169464
    andi $v0, $s1, 4
    beqz $v0, .Lvoice_80169410
    nop
.Lvoice_801693ac:
    lh $v1, 0xc($s0)
    nop
    sltiu $v0, $v1, 8
    beqz $v0, .Lvoice_80169410
    sll $v0, $v1, 2
    lui $at, %hi(audio_spu_volume_left_dispatch)
    addu $at, $at, $v0
    lw $v0, %lo(audio_spu_volume_left_dispatch)($at)
    nop
    jr $v0
    nop
.Lvoice_801693d8:
    j .Lvoice_80169418
    ori $a1, $zero, 0x8000
.Lvoice_801693e0:
    j .Lvoice_80169418
    ori $a1, $zero, 0x9000
.Lvoice_801693e8:
    j .Lvoice_80169418
    ori $a1, $zero, 0xa000
.Lvoice_801693f0:
    j .Lvoice_80169418
    ori $a1, $zero, 0xb000
.Lvoice_801693f8:
    j .Lvoice_80169418
    ori $a1, $zero, 0xc000
.Lvoice_80169400:
    j .Lvoice_80169418
    ori $a1, $zero, 0xd000
.Lvoice_80169408:
    j .Lvoice_80169418
    ori $a1, $zero, 0xe000
.Lvoice_80169410:
    lhu $s6, 8($s0)
    addu $a1, $zero, $zero
.Lvoice_80169418:
    beqz $a1, .Lvoice_8016944c
    andi $v0, $s6, 0x7fff
    lh $a0, 8($s0)
    nop
    slti $v0, $a0, 0x80
    bnez $v0, .Lvoice_8016943c
    addu $v1, $a0, $zero
    j .Lvoice_80169448
    addiu $s6, $zero, 0x7f
.Lvoice_8016943c:
    bgez $a0, .Lvoice_80169448
    addu $s6, $v1, $zero
    addu $s6, $zero, $zero
.Lvoice_80169448:
    andi $v0, $s6, 0x7fff
.Lvoice_8016944c:
    sll $v1, $s3, 1
    lui $a0, %hi(audio_spu_registers_pointer)
    lw $a0, %lo(audio_spu_registers_pointer)($a0)
    or $v0, $v0, $a1
    addu $v1, $v1, $a0
    sh $v0, 0($v1)
.Lvoice_80169464:
    bnez $s2, .Lvoice_8016947c
    andi $v0, $s1, 2
    beqz $v0, .Lvoice_80169534
    andi $v0, $s1, 8
    beqz $v0, .Lvoice_801694e0
    nop
.Lvoice_8016947c:
    lh $v1, 0xe($s0)
    nop
    sltiu $v0, $v1, 8
    beqz $v0, .Lvoice_801694e0
    sll $v0, $v1, 2
    lui $at, %hi(audio_spu_volume_right_dispatch)
    addu $at, $at, $v0
    lw $v0, %lo(audio_spu_volume_right_dispatch)($at)
    nop
    jr $v0
    nop
.Lvoice_801694a8:
    j .Lvoice_801694e8
    ori $a1, $zero, 0x8000
.Lvoice_801694b0:
    j .Lvoice_801694e8
    ori $a1, $zero, 0x9000
.Lvoice_801694b8:
    j .Lvoice_801694e8
    ori $a1, $zero, 0xa000
.Lvoice_801694c0:
    j .Lvoice_801694e8
    ori $a1, $zero, 0xb000
.Lvoice_801694c8:
    j .Lvoice_801694e8
    ori $a1, $zero, 0xc000
.Lvoice_801694d0:
    j .Lvoice_801694e8
    ori $a1, $zero, 0xd000
.Lvoice_801694d8:
    j .Lvoice_801694e8
    ori $a1, $zero, 0xe000
.Lvoice_801694e0:
    lhu $s5, 0xa($s0)
    addu $a1, $zero, $zero
.Lvoice_801694e8:
    beqz $a1, .Lvoice_8016951c
    andi $v0, $s5, 0x7fff
    lh $a0, 0xa($s0)
    nop
    slti $v0, $a0, 0x80
    bnez $v0, .Lvoice_8016950c
    addu $v1, $a0, $zero
    j .Lvoice_80169518
    addiu $s5, $zero, 0x7f
.Lvoice_8016950c:
    bgez $a0, .Lvoice_80169518
    addu $s5, $v1, $zero
    addu $s5, $zero, $zero
.Lvoice_80169518:
    andi $v0, $s5, 0x7fff
.Lvoice_8016951c:
    sll $v1, $s3, 1
    lui $a0, %hi(audio_spu_registers_pointer)
    lw $a0, %lo(audio_spu_registers_pointer)($a0)
    or $v0, $v0, $a1
    addu $v1, $v1, $a0
    sh $v0, 2($v1)
.Lvoice_80169534:
    bnez $s2, .Lvoice_80169548
    lui $v0, 1
    and $v0, $s1, $v0
    beqz $v0, .Lvoice_80169554
    nop
.Lvoice_80169548:
    lw $a1, 0x20($s0)
    jal audio_spu_write_address
    ori $a0, $s3, 7
.Lvoice_80169554:
    bnez $s2, .Lvoice_80169564
    andi $v0, $s1, 0x80
    beqz $v0, .Lvoice_80169570
    nop
.Lvoice_80169564:
    lw $a1, 0x1c($s0)
    jal audio_spu_write_address
    ori $a0, $s3, 3
.Lvoice_80169570:
    bnez $s2, .Lvoice_80169588
    andi $v0, $s1, 0x800
    beqz $v0, .Lvoice_801695f4
    andi $v0, $s1, 0x100
    beqz $v0, .Lvoice_801695ac
    addu $a2, $zero, $zero
.Lvoice_80169588:
    lw $v1, 0x24($s0)
    addiu $v0, $zero, 1
    beq $v1, $v0, .Lvoice_801695a8
    addiu $v0, $zero, 5
    bne $v1, $v0, .Lvoice_801695ac
    addu $a2, $zero, $zero
    j .Lvoice_801695ac
    addiu $a2, $zero, 0x80
.Lvoice_801695a8:
    addu $a2, $zero, $zero
.Lvoice_801695ac:
    lhu $v0, 0x30($s0)
    nop
    addu $a1, $v0, $zero
    sltiu $v0, $a1, 0x80
    bnez $v0, .Lvoice_801695c8
    nop
    addiu $a1, $zero, 0x7f
.Lvoice_801695c8:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    sll $v1, $s3, 1
    addu $v1, $v1, $v0
    lhu $v0, 8($v1)
    nop
    andi $a0, $v0, 0xff
    or $v0, $a1, $a2
    sll $v0, $v0, 8
    or $v0, $a0, $v0
    sh $v0, 8($v1)
.Lvoice_801695f4:
    bnez $s2, .Lvoice_80169604
    andi $v0, $s1, 0x1000
    beqz $v0, .Lvoice_80169648
    nop
.Lvoice_80169604:
    lhu $v0, 0x32($s0)
    nop
    addu $a1, $v0, $zero
    sltiu $v0, $a1, 0x10
    bnez $v0, .Lvoice_80169620
    nop
    addiu $a1, $zero, 0xf
.Lvoice_80169620:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    sll $v1, $s3, 1
    addu $v1, $v1, $v0
    lhu $v0, 8($v1)
    nop
    andi $a0, $v0, 0xff0f
    sll $v0, $a1, 4
    or $v0, $a0, $v0
    sh $v0, 8($v1)
.Lvoice_80169648:
    bnez $s2, .Lvoice_80169658
    andi $v0, $s1, 0x8000
    beqz $v0, .Lvoice_80169698
    nop
.Lvoice_80169658:
    lhu $v0, 0x38($s0)
    nop
    addu $a1, $v0, $zero
    sltiu $v0, $a1, 0x10
    bnez $v0, .Lvoice_80169674
    nop
    addiu $a1, $zero, 0xf
.Lvoice_80169674:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    sll $v1, $s3, 1
    addu $v1, $v1, $v0
    lhu $v0, 8($v1)
    nop
    andi $v0, $v0, 0xfff0
    or $v0, $a1, $v0
    sh $v0, 8($v1)
.Lvoice_80169698:
    bnez $s2, .Lvoice_801696b0
    andi $v0, $s1, 0x2000
    beqz $v0, .Lvoice_80169748
    andi $v0, $s1, 0x200
    beqz $v0, .Lvoice_80169700
    addu $a2, $zero, $zero
.Lvoice_801696b0:
    lw $v1, 0x28($s0)
    addiu $v0, $zero, 3
    beq $v1, $v0, .Lvoice_801696fc
    slti $v0, $v1, 4
    beqz $v0, .Lvoice_801696d8
    addiu $v0, $zero, 1
    beq $v1, $v0, .Lvoice_80169700
    addu $a2, $zero, $zero
    j .Lvoice_80169700
    addiu $a2, $zero, 0x100
.Lvoice_801696d8:
    addiu $v0, $zero, 5
    beq $v1, $v0, .Lvoice_801696f4
    addiu $v0, $zero, 7
    beq $v1, $v0, .Lvoice_80169700
    addiu $a2, $zero, 0x300
    j .Lvoice_80169700
    addiu $a2, $zero, 0x100
.Lvoice_801696f4:
    j .Lvoice_80169700
    addiu $a2, $zero, 0x200
.Lvoice_801696fc:
    addiu $a2, $zero, 0x100
.Lvoice_80169700:
    lhu $v0, 0x34($s0)
    nop
    addu $a1, $v0, $zero
    sltiu $v0, $a1, 0x80
    bnez $v0, .Lvoice_8016971c
    nop
    addiu $a1, $zero, 0x7f
.Lvoice_8016971c:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    sll $v1, $s3, 1
    addu $v1, $v1, $v0
    lhu $v0, 0xa($v1)
    nop
    andi $a0, $v0, 0x3f
    or $v0, $a1, $a2
    sll $v0, $v0, 6
    or $v0, $a0, $v0
    sh $v0, 0xa($v1)
.Lvoice_80169748:
    bnez $s2, .Lvoice_80169760
    andi $v0, $s1, 0x4000
    beqz $v0, .Lvoice_801697c8
    andi $v0, $s1, 0x400
    beqz $v0, .Lvoice_80169784
    addu $a2, $zero, $zero
.Lvoice_80169760:
    lw $v1, 0x2c($s0)
    addiu $v0, $zero, 3
    beq $v1, $v0, .Lvoice_80169780
    addiu $v0, $zero, 7
    bne $v1, $v0, .Lvoice_80169784
    addu $a2, $zero, $zero
    j .Lvoice_80169784
    addiu $a2, $zero, 0x20
.Lvoice_80169780:
    addu $a2, $zero, $zero
.Lvoice_80169784:
    lhu $v0, 0x36($s0)
    nop
    addu $a1, $v0, $zero
    sltiu $v0, $a1, 0x20
    bnez $v0, .Lvoice_801697a0
    nop
    addiu $a1, $zero, 0x1f
.Lvoice_801697a0:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    sll $v1, $s3, 1
    addu $v1, $v1, $v0
    lhu $v0, 0xa($v1)
    nop
    andi $a0, $v0, 0xffc0
    or $v0, $a1, $a2
    or $v0, $a0, $v0
    sh $v0, 0xa($v1)
.Lvoice_801697c8:
    addiu $s4, $s4, 1
    slt $v0, $s4, $s7
    bnez $v0, .Lvoice_8016928c
    addiu $v0, $zero, 1
.Lvoice_801697d8:
    bnez $fp, .Lvoice_80169838
    addu $v0, $zero, $zero
    addiu $v0, $zero, 1
    sw $v0, 0x14($sp)
    sw $zero, 0x10($sp)
    j .Lvoice_80169824
    nop
.Lvoice_801697f4:
    lw $v1, 0x14($sp)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sw $v0, 0x14($sp)
    lw $v0, 0x10($sp)
    nop
    addiu $v0, $v0, 1
    sw $v0, 0x10($sp)
    lw $v0, 0x10($sp)
.Lvoice_80169824:
    lw $v0, 0x10($sp)
    nop
    slti $v0, $v0, 2
    bnez $v0, .Lvoice_801697f4
    addu $v0, $zero, $zero
.Lvoice_80169838:
    lw $ra, 0x4c($sp)
    lw $fp, 0x48($sp)
    lw $s7, 0x44($sp)
    lw $s6, 0x40($sp)
    lw $s5, 0x3c($sp)
    lw $s4, 0x38($sp)
    lw $s3, 0x34($sp)
    lw $s2, 0x30($sp)
    lw $s1, 0x2c($sp)
    lw $s0, 0x28($sp)
    addiu $sp, $sp, 0x50
    jr $ra
    nop
.end audio_spu_set_voice_range

# exe/slus_004_22@0x8016986C..0x8016995C
.globl audio_spu_note_to_pitch
.ent audio_spu_note_to_pitch
audio_spu_note_to_pitch:
    andi $a3, $a3, 0xffff
    andi $a1, $a1, 0xffff
    addu $v0, $a3, $a1
    bgez $v0, .Lvoice_80169884
    addu $t0, $a0, $zero
    addiu $v0, $v0, 7
.Lvoice_80169884:
    sra $v0, $v0, 3
    addu $a1, $v0, $zero
    slti $v0, $v0, 0x10
    bnez $v0, .Lvoice_801698b4
    addu $v1, $zero, $zero
.Lvoice_80169898:
    addiu $v0, $a1, -0x10
    addu $a1, $v0, $zero
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x10
    beqz $v0, .Lvoice_80169898
    addiu $v1, $v1, 1
.Lvoice_801698b4:
    sll $v0, $a1, 0x10
    bgez $v0, .Lvoice_801698c4
    lui $a0, 0x2aaa
    addu $a1, $zero, $zero
.Lvoice_801698c4:
    ori $a0, $a0, 0xaaab
    addiu $v0, $a2, 0x3c
    subu $v0, $v0, $t0
    addu $v0, $v1, $v0
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
    sll $v0, $a1, 0x10
    sra $v0, $v0, 0x10
    addu $v1, $v1, $v0
    sll $v1, $v1, 1
    lui $at, %hi(audio_spu_pitch_table)
    addu $at, $at, $v1
    lhu $v1, %lo(audio_spu_pitch_table)($at)
    addiu $a0, $a0, -5
    sll $a0, $a0, 0x10
    sra $v0, $a0, 0x10
    blez $v0, .Lvoice_80169940
    nop
    j .Lvoice_80169954
    sllv $v1, $v1, $v0
.Lvoice_80169940:
    bgez $v0, .Lvoice_80169954
    nop
    andi $v1, $v1, 0xffff
    negu $v0, $v0
    srav $v1, $v1, $v0
.Lvoice_80169954:
    jr $ra
    andi $v0, $v1, 0xffff
.end audio_spu_note_to_pitch

.section .data
.balign 4
# exe/slus_004_22@0x8014A008, eight bounded entries.
.globl audio_spu_volume_left_dispatch
audio_spu_volume_left_dispatch:
    .word .Lvoice_80169410
    .word .Lvoice_801693d8
    .word .Lvoice_801693e0
    .word .Lvoice_801693e8
    .word .Lvoice_801693f0
    .word .Lvoice_801693f8
    .word .Lvoice_80169400
    .word .Lvoice_80169408
# exe/slus_004_22@0x8014A028, eight bounded entries.
.globl audio_spu_volume_right_dispatch
audio_spu_volume_right_dispatch:
    .word .Lvoice_801694e0
    .word .Lvoice_801694a8
    .word .Lvoice_801694b0
    .word .Lvoice_801694b8
    .word .Lvoice_801694c0
    .word .Lvoice_801694c8
    .word .Lvoice_801694d0
    .word .Lvoice_801694d8

.balign 4
# Ten reverb presets of 68 bytes precede the 193-halfword pitch table.
# Keep these labels adjacent. Pitch indices -176..-1 read the preset tail.
# exe/slus_004_22@0x80183FDC, 680 bytes.
.globl audio_spu_reverb_presets
audio_spu_reverb_presets:
    .half 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    .half 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    .half 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    .half 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    .half 0x0000, 0x0000, 0x0000, 0x0000, 0x007d, 0x005b, 0x6d80, 0x54b8
    .half 0xbed0, 0x0000, 0x0000, 0xba80, 0x5800, 0x5300, 0x04d6, 0x0333
    .half 0x03f0, 0x0227, 0x0374, 0x01ef, 0x0334, 0x01b5, 0x0000, 0x0000
    .half 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x01b4, 0x0136
    .half 0x00b8, 0x005c, 0x8000, 0x8000, 0x0000, 0x0000, 0x0033, 0x0025
    .half 0x70f0, 0x4fa8, 0xbce0, 0x4410, 0xc0f0, 0x9c00, 0x5280, 0x4ec0
    .half 0x03e4, 0x031b, 0x03a4, 0x02af, 0x0372, 0x0266, 0x031c, 0x025d
    .half 0x025c, 0x018e, 0x022f, 0x0135, 0x01d2, 0x00b7, 0x018f, 0x00b5
    .half 0x00b4, 0x0080, 0x004c, 0x0026, 0x8000, 0x8000, 0x0000, 0x0000
    .half 0x00b1, 0x007f, 0x70f0, 0x4fa8, 0xbce0, 0x4510, 0xbef0, 0xb4c0
    .half 0x5280, 0x4ec0, 0x0904, 0x076b, 0x0824, 0x065f, 0x07a2, 0x0616
    .half 0x076c, 0x05ed, 0x05ec, 0x042e, 0x050f, 0x0305, 0x0462, 0x02b7
    .half 0x042f, 0x0265, 0x0264, 0x01b2, 0x0100, 0x0080, 0x8000, 0x8000
    .half 0x0000, 0x0000, 0x00e3, 0x00a9, 0x6f60, 0x4fa8, 0xbce0, 0x4510
    .half 0xbef0, 0xa680, 0x5680, 0x52c0, 0x0dfb, 0x0b58, 0x0d09, 0x0a3c
    .half 0x0bd9, 0x0973, 0x0b59, 0x08da, 0x08d9, 0x05e9, 0x07ec, 0x04b0
    .half 0x06ef, 0x03d2, 0x05ea, 0x031d, 0x031c, 0x0238, 0x0154, 0x00aa
    .half 0x8000, 0x8000, 0x0000, 0x0000, 0x01a5, 0x0139, 0x6000, 0x5000
    .half 0x4c00, 0xb800, 0xbc00, 0xc000, 0x6000, 0x5c00, 0x15ba, 0x11bb
    .half 0x14c2, 0x10bd, 0x11bc, 0x0dc1, 0x11c0, 0x0dc3, 0x0dc0, 0x09c1
    .half 0x0bc4, 0x07c1, 0x0a00, 0x06cd, 0x09c2, 0x05c1, 0x05c0, 0x041a
    .half 0x0274, 0x013a, 0x8000, 0x8000, 0x0000, 0x0000, 0x033d, 0x0231
    .half 0x7e00, 0x5000, 0xb400, 0xb000, 0x4c00, 0xb000, 0x6000, 0x5400
    .half 0x1ed6, 0x1a31, 0x1d14, 0x183b, 0x1bc2, 0x16b2, 0x1a32, 0x15ef
    .half 0x15ee, 0x1055, 0x1334, 0x0f2d, 0x11f6, 0x0c5d, 0x1056, 0x0ae1
    .half 0x0ae0, 0x07a2, 0x0464, 0x0232, 0x8000, 0x8000, 0x0000, 0x0000
    .half 0x0001, 0x0001, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x8100
    .half 0x0000, 0x0000, 0x1fff, 0x0fff, 0x1005, 0x0005, 0x0000, 0x0000
    .half 0x1005, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    .half 0x0000, 0x0000, 0x1004, 0x1002, 0x0004, 0x0002, 0x8000, 0x8000
    .half 0x0000, 0x0000, 0x0001, 0x0001, 0x7fff, 0x7fff, 0x0000, 0x0000
    .half 0x0000, 0x0000, 0x0000, 0x0000, 0x1fff, 0x0fff, 0x1005, 0x0005
    .half 0x0000, 0x0000, 0x1005, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000
    .half 0x0000, 0x0000, 0x0000, 0x0000, 0x1004, 0x1002, 0x0004, 0x0002
    .half 0x8000, 0x8000, 0x0000, 0x0000, 0x0017, 0x0013, 0x70f0, 0x4fa8
    .half 0xbce0, 0x4510, 0xbef0, 0x8500, 0x5f80, 0x54c0, 0x0371, 0x02af
    .half 0x02e5, 0x01df, 0x02b0, 0x01d7, 0x0358, 0x026a, 0x01d6, 0x011e
    .half 0x012d, 0x00b1, 0x011f, 0x0059, 0x01a0, 0x00e3, 0x0058, 0x0040
    .half 0x0028, 0x0014, 0x8000, 0x8000
.globl audio_spu_reverb_presets_end
audio_spu_reverb_presets_end:
# exe/slus_004_22@0x80184284, 386 bytes.
.globl audio_spu_pitch_table
audio_spu_pitch_table:
    .half 0x1000, 0x100e, 0x101d, 0x102c, 0x103b, 0x104a, 0x1059, 0x1068
    .half 0x1078, 0x1087, 0x1096, 0x10a5, 0x10b5, 0x10c4, 0x10d4, 0x10e3
    .half 0x10f3, 0x1103, 0x1113, 0x1122, 0x1132, 0x1142, 0x1152, 0x1162
    .half 0x1172, 0x1182, 0x1193, 0x11a3, 0x11b3, 0x11c4, 0x11d4, 0x11e5
    .half 0x11f5, 0x1206, 0x1216, 0x1227, 0x1238, 0x1249, 0x125a, 0x126b
    .half 0x127c, 0x128d, 0x129e, 0x12af, 0x12c1, 0x12d2, 0x12e3, 0x12f5
    .half 0x1306, 0x1318, 0x132a, 0x133c, 0x134d, 0x135f, 0x1371, 0x1383
    .half 0x1395, 0x13a7, 0x13ba, 0x13cc, 0x13de, 0x13f1, 0x1403, 0x1416
    .half 0x1428, 0x143b, 0x144e, 0x1460, 0x1473, 0x1486, 0x1499, 0x14ac
    .half 0x14bf, 0x14d3, 0x14e6, 0x14f9, 0x150d, 0x1520, 0x1534, 0x1547
    .half 0x155b, 0x156f, 0x1583, 0x1597, 0x15ab, 0x15bf, 0x15d3, 0x15e7
    .half 0x15fb, 0x1610, 0x1624, 0x1638, 0x164d, 0x1662, 0x1676, 0x168b
    .half 0x16a0, 0x16b5, 0x16ca, 0x16df, 0x16f4, 0x170a, 0x171f, 0x1734
    .half 0x174a, 0x175f, 0x1775, 0x178b, 0x17a1, 0x17b6, 0x17cc, 0x17e2
    .half 0x17f9, 0x180f, 0x1825, 0x183b, 0x1852, 0x1868, 0x187f, 0x1896
    .half 0x18ac, 0x18c3, 0x18da, 0x18f1, 0x1908, 0x191f, 0x1937, 0x194e
    .half 0x1965, 0x197d, 0x1995, 0x19ac, 0x19c4, 0x19dc, 0x19f4, 0x1a0c
    .half 0x1a24, 0x1a3c, 0x1a55, 0x1a6d, 0x1a85, 0x1a9e, 0x1ab7, 0x1acf
    .half 0x1ae8, 0x1b01, 0x1b1a, 0x1b33, 0x1b4c, 0x1b66, 0x1b7f, 0x1b98
    .half 0x1bb2, 0x1bcc, 0x1be5, 0x1bff, 0x1c19, 0x1c33, 0x1c4d, 0x1c67
    .half 0x1c82, 0x1c9c, 0x1cb7, 0x1cd1, 0x1cec, 0x1d07, 0x1d22, 0x1d3d
    .half 0x1d58, 0x1d73, 0x1d8e, 0x1da9, 0x1dc5, 0x1de0, 0x1dfc, 0x1e18
    .half 0x1e34, 0x1e50, 0x1e6c, 0x1e88, 0x1ea4, 0x1ec1, 0x1edd, 0x1efa
    .half 0x1f16, 0x1f33, 0x1f50, 0x1f6d, 0x1f8a, 0x1fa7, 0x1fc5, 0x1fe2
    .half 0x2000
.globl audio_spu_pitch_table_end
audio_spu_pitch_table_end:

# Keep both pitch tables distinct and preserve the writable neighbors read by
# the original note helper. The word at 0x80184448 is retained as numeric data.
# exe/slus_004_22@0x80184406, 10 bytes.
.globl audio_sequence_pitch_adjacent_data
audio_sequence_pitch_adjacent_data:
    .byte 0x00, 0x00, 0x50, 0x73, 0x10, 0x0c, 0xda, 0xc1, 0x37, 0x00
# exe/slus_004_22@0x80184410, 48 bytes.
.globl audio_library_register_defaults
audio_library_register_defaults:
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x30, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    .byte 0xff, 0x3f, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
# exe/slus_004_22@0x80184440, 4 bytes.
.globl audio_sequence_tick_mode
audio_sequence_tick_mode:
    .byte 0x3c, 0x00, 0x00, 0x00
# exe/slus_004_22@0x80184444, 4 bytes.
.globl audio_sequence_tick_flag
audio_sequence_tick_flag:
    .byte 0x01, 0x00, 0x00, 0x00
# exe/slus_004_22@0x80184448, 16 bytes.
.globl audio_sequence_pitch_control_words
audio_sequence_pitch_control_words:
    .byte 0x48, 0xc5, 0x16, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00
# exe/slus_004_22@0x80184458, 4 bytes.
.globl audio_sequence_registers_pointer
audio_sequence_registers_pointer:
    .byte 0x00, 0x1c, 0x80, 0x1f
# exe/slus_004_22@0x8018445C, 386 bytes.
.globl audio_sequence_pitch_table
audio_sequence_pitch_table:
    .byte 0x00, 0x10, 0x0e, 0x10, 0x1d, 0x10, 0x2c, 0x10, 0x3b, 0x10, 0x4a, 0x10, 0x59, 0x10, 0x68, 0x10
    .byte 0x78, 0x10, 0x87, 0x10, 0x96, 0x10, 0xa5, 0x10, 0xb5, 0x10, 0xc4, 0x10, 0xd4, 0x10, 0xe3, 0x10
    .byte 0xf3, 0x10, 0x03, 0x11, 0x13, 0x11, 0x22, 0x11, 0x32, 0x11, 0x42, 0x11, 0x52, 0x11, 0x62, 0x11
    .byte 0x72, 0x11, 0x82, 0x11, 0x93, 0x11, 0xa3, 0x11, 0xb3, 0x11, 0xc4, 0x11, 0xd4, 0x11, 0xe5, 0x11
    .byte 0xf5, 0x11, 0x06, 0x12, 0x16, 0x12, 0x27, 0x12, 0x38, 0x12, 0x49, 0x12, 0x5a, 0x12, 0x6b, 0x12
    .byte 0x7c, 0x12, 0x8d, 0x12, 0x9e, 0x12, 0xaf, 0x12, 0xc1, 0x12, 0xd2, 0x12, 0xe3, 0x12, 0xf5, 0x12
    .byte 0x06, 0x13, 0x18, 0x13, 0x2a, 0x13, 0x3c, 0x13, 0x4d, 0x13, 0x5f, 0x13, 0x71, 0x13, 0x83, 0x13
    .byte 0x95, 0x13, 0xa7, 0x13, 0xba, 0x13, 0xcc, 0x13, 0xde, 0x13, 0xf1, 0x13, 0x03, 0x14, 0x16, 0x14
    .byte 0x28, 0x14, 0x3b, 0x14, 0x4e, 0x14, 0x60, 0x14, 0x73, 0x14, 0x86, 0x14, 0x99, 0x14, 0xac, 0x14
    .byte 0xbf, 0x14, 0xd3, 0x14, 0xe6, 0x14, 0xf9, 0x14, 0x0d, 0x15, 0x20, 0x15, 0x34, 0x15, 0x47, 0x15
    .byte 0x5b, 0x15, 0x6f, 0x15, 0x83, 0x15, 0x97, 0x15, 0xab, 0x15, 0xbf, 0x15, 0xd3, 0x15, 0xe7, 0x15
    .byte 0xfb, 0x15, 0x10, 0x16, 0x24, 0x16, 0x38, 0x16, 0x4d, 0x16, 0x62, 0x16, 0x76, 0x16, 0x8b, 0x16
    .byte 0xa0, 0x16, 0xb5, 0x16, 0xca, 0x16, 0xdf, 0x16, 0xf4, 0x16, 0x0a, 0x17, 0x1f, 0x17, 0x34, 0x17
    .byte 0x4a, 0x17, 0x5f, 0x17, 0x75, 0x17, 0x8b, 0x17, 0xa1, 0x17, 0xb6, 0x17, 0xcc, 0x17, 0xe2, 0x17
    .byte 0xf9, 0x17, 0x0f, 0x18, 0x25, 0x18, 0x3b, 0x18, 0x52, 0x18, 0x68, 0x18, 0x7f, 0x18, 0x96, 0x18
    .byte 0xac, 0x18, 0xc3, 0x18, 0xda, 0x18, 0xf1, 0x18, 0x08, 0x19, 0x1f, 0x19, 0x37, 0x19, 0x4e, 0x19
    .byte 0x65, 0x19, 0x7d, 0x19, 0x95, 0x19, 0xac, 0x19, 0xc4, 0x19, 0xdc, 0x19, 0xf4, 0x19, 0x0c, 0x1a
    .byte 0x24, 0x1a, 0x3c, 0x1a, 0x55, 0x1a, 0x6d, 0x1a, 0x85, 0x1a, 0x9e, 0x1a, 0xb7, 0x1a, 0xcf, 0x1a
    .byte 0xe8, 0x1a, 0x01, 0x1b, 0x1a, 0x1b, 0x33, 0x1b, 0x4c, 0x1b, 0x66, 0x1b, 0x7f, 0x1b, 0x98, 0x1b
    .byte 0xb2, 0x1b, 0xcc, 0x1b, 0xe5, 0x1b, 0xff, 0x1b, 0x19, 0x1c, 0x33, 0x1c, 0x4d, 0x1c, 0x67, 0x1c
    .byte 0x82, 0x1c, 0x9c, 0x1c, 0xb7, 0x1c, 0xd1, 0x1c, 0xec, 0x1c, 0x07, 0x1d, 0x22, 0x1d, 0x3d, 0x1d
    .byte 0x58, 0x1d, 0x73, 0x1d, 0x8e, 0x1d, 0xa9, 0x1d, 0xc5, 0x1d, 0xe0, 0x1d, 0xfc, 0x1d, 0x18, 0x1e
    .byte 0x34, 0x1e, 0x50, 0x1e, 0x6c, 0x1e, 0x88, 0x1e, 0xa4, 0x1e, 0xc1, 0x1e, 0xdd, 0x1e, 0xfa, 0x1e
    .byte 0x16, 0x1f, 0x33, 0x1f, 0x50, 0x1f, 0x6d, 0x1f, 0x8a, 0x1f, 0xa7, 0x1f, 0xc5, 0x1f, 0xe2, 0x1f
    .byte 0x00, 0x20
# exe/slus_004_22@0x801845DE, 42 bytes.
.globl audio_sequence_pitch_suffix
audio_sequence_pitch_suffix:
    .byte 0x00, 0x00
.globl audio_bank_transfer_progress
audio_bank_transfer_progress:
    .byte 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00
    .byte 0x80, 0x00, 0x50, 0x73, 0x06, 0x0c, 0xda, 0x9b, 0x37, 0x00, 0x14, 0x18, 0x80, 0x1f, 0x10, 0x11
    .byte 0x80, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
