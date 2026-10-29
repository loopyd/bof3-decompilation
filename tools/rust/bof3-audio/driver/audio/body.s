# SPDX-License-Identifier: MIT
# Owned VAB body upload and transfer completion routines.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Transfer state is shared with the owned SPU and bank routines.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x80174354..0x801744B4
.globl audio_bank_transfer_partial
.ent audio_bank_transfer_partial
audio_bank_transfer_partial:
    addiu $sp, $sp, -0x28
    sw $s4, 0x20($sp)
    addu $s4, $a0, $zero
    sw $s3, 0x1c($sp)
    addu $s3, $a1, $zero
    addu $a1, $a2, $zero
    andi $v0, $a2, 0xffff
    sltiu $v0, $v0, 0x11
    sw $ra, 0x24($sp)
    sw $s2, 0x18($sp)
    sw $s1, 0x14($sp)
    beqz $v0, .Lbody_80174408
    sw $s0, 0x10($sp)
    sll $v0, $a2, 0x10
    sra $s1, $v0, 0x10
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $s1
    lbu $v1, %lo(audio_sequence_voice_flags)($at)
    addiu $v0, $zero, 2
    bne $v1, $v0, .Lbody_80174408
    nop
    lui $v0, %hi(audio_bank_transfer_progress)
    lw $v0, %lo(audio_bank_transfer_progress)($v0)
    nop
    bnez $v0, .Lbody_801743f4
    sll $s0, $s1, 2
    lui $at, %hi(audio_bank_body_sizes)
    addu $at, $at, $s0
    lw $v0, %lo(audio_bank_body_sizes)($at)
    lui $at, %hi(audio_bank_transfer_progress + 0x4)
    sh $a1, %lo(audio_bank_transfer_progress + 0x4)($at)
    lui $at, %hi(audio_bank_transfer_progress)
    sw $v0, %lo(audio_bank_transfer_progress)($at)
    jal .Lbody_801690b8
    addu $a0, $zero, $zero
    lui $at, %hi(audio_bank_spu_bases)
    addu $at, $at, $s0
    lw $a0, %lo(audio_bank_spu_bases)($at)
    jal .Lbody_801744b4
    nop
.Lbody_801743f4:
    lui $s2, %hi(audio_bank_transfer_progress + 0x4)
    lh $s2, %lo(audio_bank_transfer_progress + 0x4)($s2)
    nop
    beq $s2, $s1, .Lbody_80174418
    addu $s0, $s3, $zero
.Lbody_80174408:
    jal audio_spu_set_transfer_mode
    addu $a0, $zero, $zero
    j .Lbody_80174490
    addiu $v0, $zero, -1
.Lbody_80174418:
    lui $v1, %hi(audio_bank_transfer_progress)
    lw $v1, %lo(audio_bank_transfer_progress)($v1)
    nop
    sltu $v0, $v1, $s0
    beqz $v0, .Lbody_80174434
    nop
    addu $s0, $v1, $zero
.Lbody_80174434:
    jal audio_spu_set_transfer_mode
    addiu $a0, $zero, 1
    addu $a0, $s4, $zero
    jal .Lbody_8017450c
    addu $a1, $s0, $zero
    lui $v0, %hi(audio_bank_transfer_progress)
    lw $v0, %lo(audio_bank_transfer_progress)($v0)
    nop
    subu $v0, $v0, $s0
    lui $at, %hi(audio_bank_transfer_progress)
    sw $v0, %lo(audio_bank_transfer_progress)($at)
    bnez $v0, .Lbody_80174490
    addiu $v0, $zero, -2
    addu $v0, $s2, $zero
    addiu $v1, $zero, -1
    lui $at, %hi(audio_bank_transfer_progress + 0x4)
    sh $v1, %lo(audio_bank_transfer_progress + 0x4)($at)
    addiu $v1, $zero, 1
    lui $at, %hi(audio_bank_transfer_progress)
    sw $zero, %lo(audio_bank_transfer_progress)($at)
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $v0
    sb $v1, %lo(audio_sequence_voice_flags)($at)
.Lbody_80174490:
    lw $ra, 0x24($sp)
    lw $s4, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x28
    jr $ra
    nop
.end audio_bank_transfer_partial

# exe/slus_004_22@0x801744B4..0x8017450C
.globl audio_spu_set_transfer_address
.ent audio_spu_set_transfer_address
audio_spu_set_transfer_address:
.Lbody_801744b4:
    addiu $sp, $sp, -0x18
    addu $a1, $a0, $zero
    lui $v0, 7
    ori $v0, $v0, 0xefe8
    addiu $v1, $a1, -0x1010
    sltu $v0, $v0, $v1
    bnez $v0, .Lbody_801744f8
    sw $ra, 0x10($sp)
    jal audio_spu_write_address
    addiu $a0, $zero, -1
    lui $v1, %hi(audio_spu_transfer_shift)
    lw $v1, %lo(audio_spu_transfer_shift)($v1)
    lui $at, %hi(audio_spu_transfer_address)
    sh $v0, %lo(audio_spu_transfer_address)($at)
    andi $v0, $v0, 0xffff
    j .Lbody_801744fc
    sllv $v0, $v0, $v1
.Lbody_801744f8:
    addu $v0, $zero, $zero
.Lbody_801744fc:
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_spu_set_transfer_address

# exe/slus_004_22@0x8017450C..0x80174598
.globl audio_spu_write
.ent audio_spu_write
audio_spu_write:
.Lbody_8017450c:
    addiu $sp, $sp, -0x20
    sw $s1, 0x14($sp)
    addu $s1, $a1, $zero
    lui $v0, 7
    ori $v0, $v0, 0xeff0
    sltu $v0, $v0, $s1
    sw $ra, 0x18($sp)
    beqz $v0, .Lbody_80174538
    sw $s0, 0x10($sp)
    lui $s1, 7
    ori $s1, $s1, 0xeff0
.Lbody_80174538:
    lui $s0, %hi(audio_spu_transfer_address)
    lhu $s0, %lo(audio_spu_transfer_address)($s0)
    lui $v0, %hi(audio_spu_transfer_shift)
    lw $v0, %lo(audio_spu_transfer_shift)($v0)
    addu $a1, $s1, $zero
    jal .Lbody_80168bf0
    sllv $s0, $s0, $v0
    addiu $a0, $zero, -1
    jal audio_spu_write_address
    addu $a1, $s0, $s1
    lui $v1, %hi(audio_spu_transfer_callback)
    lw $v1, %lo(audio_spu_transfer_callback)($v1)
    lui $at, %hi(audio_spu_transfer_address)
    sh $v0, %lo(audio_spu_transfer_address)($at)
    bnez $v1, .Lbody_80174580
    addu $v0, $s1, $zero
    lui $at, %hi(audio_spu_state_3ae4)
    sw $zero, %lo(audio_spu_state_3ae4)($at)
.Lbody_80174580:
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_spu_write

# exe/slus_004_22@0x80174598..0x801745C0
.globl audio_bank_transfer_completed
.ent audio_bank_transfer_completed
audio_bank_transfer_completed:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    sll $a0, $a0, 0x10
    jal .Lbody_801745c0
    sra $a0, $a0, 0x10
    sll $v0, $v0, 0x10
    lw $ra, 0x10($sp)
    sra $v0, $v0, 0x10
    jr $ra
    addiu $sp, $sp, 0x18
.end audio_bank_transfer_completed

# exe/slus_004_22@0x801745C0..0x80174668
.globl audio_spu_transfer_completed
.ent audio_spu_transfer_completed
audio_spu_transfer_completed:
.Lbody_801745c0:
    lui $v0, %hi(audio_spu_state_3b1c)
    lw $v0, %lo(audio_spu_state_3b1c)($v0)
    addiu $sp, $sp, -0x20
    sw $s1, 0x14($sp)
    addu $s1, $a0, $zero
    sw $s0, 0x10($sp)
    addiu $s0, $zero, 1
    beq $v0, $s0, .Lbody_801745f8
    sw $ra, 0x18($sp)
    lui $v0, %hi(audio_spu_state_3ae4)
    lw $v0, %lo(audio_spu_state_3ae4)($v0)
    nop
    bne $v0, $s0, .Lbody_80174600
    nop
.Lbody_801745f8:
    j .Lbody_80174650
    addiu $v0, $zero, 1
.Lbody_80174600:
    lui $a0, %hi(audio_spu_transfer_event)
    lw $a0, %lo(audio_spu_transfer_event)($a0)
    jal .Lbody_8017ed6c
    nop
    bne $s1, $s0, .Lbody_80174640
    nop
    bnez $v0, .Lbody_80174648
    addiu $v0, $zero, 1
.Lbody_80174620:
    lui $a0, %hi(audio_spu_transfer_event)
    lw $a0, %lo(audio_spu_transfer_event)($a0)
    jal .Lbody_8017ed6c
    nop
    beqz $v0, .Lbody_80174620
    addiu $v0, $zero, 1
    j .Lbody_80174648
    nop
.Lbody_80174640:
    bne $v0, $s0, .Lbody_80174650
    nop
.Lbody_80174648:
    lui $at, %hi(audio_spu_state_3ae4)
    sw $v0, %lo(audio_spu_state_3ae4)($at)
.Lbody_80174650:
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_spu_transfer_completed

# exe/slus_004_22@0x80168BF0..0x80168C78
.globl audio_spu_write_dispatch
.ent audio_spu_write_dispatch
audio_spu_write_dispatch:
.Lbody_80168bf0:
    lui $v0, %hi(audio_spu_state_3acc)
    lw $v0, %lo(audio_spu_state_3acc)($v0)
    addiu $sp, $sp, -0x20
    sw $s1, 0x14($sp)
    addu $s1, $a0, $zero
    sw $s0, 0x10($sp)
    addu $s0, $a1, $zero
    bnez $v0, .Lbody_80168c50
    sw $ra, 0x18($sp)
    lui $v0, %hi(audio_spu_transfer_address)
    lhu $v0, %lo(audio_spu_transfer_address)($v0)
    lui $a1, %hi(audio_spu_transfer_shift)
    lw $a1, %lo(audio_spu_transfer_shift)($a1)
    addiu $a0, $zero, 2
    jal audio_spu_transfer
    sllv $a1, $v0, $a1
    jal audio_spu_transfer
    addiu $a0, $zero, 1
    addiu $a0, $zero, 3
    addu $a1, $s1, $zero
    jal audio_spu_transfer
    addu $a2, $s0, $zero
    j .Lbody_80168c60
    addu $v0, $s0, $zero
.Lbody_80168c50:
    addu $a0, $s1, $zero
    jal audio_spu_write_fifo
    addu $a1, $s0, $zero
    addu $v0, $s0, $zero
.Lbody_80168c60:
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_spu_write_dispatch

# exe/slus_004_22@0x801690B8..0x801690EC
.globl audio_spu_configure_transfer_method
.ent audio_spu_configure_transfer_method
audio_spu_configure_transfer_method:
.Lbody_801690b8:
    beqz $a0, .Lbody_801690d0
    addiu $v0, $zero, 1
    bne $a0, $v0, .Lbody_801690d4
    addu $v0, $zero, $zero
    j .Lbody_801690d4
    addiu $v0, $zero, 1
.Lbody_801690d0:
    addu $v0, $zero, $zero
.Lbody_801690d4:
    lui $at, %hi(audio_spu_state_3b1c)
    sw $a0, %lo(audio_spu_state_3b1c)($at)
    lui $at, %hi(audio_spu_state_3acc)
    sw $v0, %lo(audio_spu_state_3acc)($at)
    jr $ra
    nop
.end audio_spu_configure_transfer_method

# exe/slus_004_22@0x8017ED6C..0x8017ED7C
.globl audio_body_test_event
.ent audio_body_test_event
audio_body_test_event:
.Lbody_8017ed6c:
    addiu $t2, $zero, 0xb0
    jr $t2
    addiu $t1, $zero, 0xb
    nop
.end audio_body_test_event
