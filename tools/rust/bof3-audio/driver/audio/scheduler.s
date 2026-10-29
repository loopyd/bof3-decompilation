# SPDX-License-Identifier: MIT
# Owned sequence callback registration, scheduling and event consumption.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Records, event streams and callback cells remain live caller-owned state.
.set noreorder
.set noat

.section .text
.balign 4
# exe/slus_004_22@0x8015CE70..0x8015CEBC
.globl audio_music_start_callbacks
.ent audio_music_start_callbacks
audio_music_start_callbacks:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_sequence_start_callbacks
    nop
    addiu $a0, $zero, 0x7f
    jal audio_spu_set_master_volume
    addiu $a1, $zero, 0x7f
    addu $a0, $zero, $zero
    addu $a1, $zero, $zero
    jal audio_spu_set_output_switch
    addiu $a2, $zero, 1
    addu $a0, $zero, $zero
    addiu $a1, $zero, 0x6e
    jal audio_sequence_set_output_gain
    addiu $a2, $zero, 0x6e
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_music_start_callbacks

.section .text
.balign 4
# exe/slus_004_22@0x8016BC98..0x8016BD2C
.globl audio_spu_set_output_switch
.ent audio_spu_set_output_switch
audio_spu_set_output_switch:
    addiu $sp, $sp, -0x40
    andi $v0, $a0, 0xff
    bnez $v0, .Lscheduler_8016bcdc
    sw $ra, 0x38($sp)
    andi $v1, $a1, 0xff
    bnez $v1, .Lscheduler_8016bcc8
    addiu $v0, $zero, 1
    addiu $v0, $zero, 0x200
    sw $v0, 0x10($sp)
    andi $v0, $a2, 0xff
    sw $v0, 0x28($sp)
    addiu $v0, $zero, 1
.Lscheduler_8016bcc8:
    bne $v1, $v0, .Lscheduler_8016bcdc
    addiu $v0, $zero, 0x100
    sw $v0, 0x10($sp)
    andi $v0, $a2, 0xff
    sw $v0, 0x24($sp)
.Lscheduler_8016bcdc:
    andi $a0, $a0, 0xff
    addiu $v0, $zero, 1
    bne $a0, $v0, .Lscheduler_8016bd14
    andi $a1, $a1, 0xff
    bnez $a1, .Lscheduler_8016bd00
    addiu $v0, $zero, 0x2000
    sw $v0, 0x10($sp)
    andi $v0, $a2, 0xff
    sw $v0, 0x34($sp)
.Lscheduler_8016bd00:
    bne $a1, $a0, .Lscheduler_8016bd14
    addiu $v0, $zero, 0x1000
    sw $v0, 0x10($sp)
    andi $v0, $a2, 0xff
    sw $v0, 0x30($sp)
.Lscheduler_8016bd14:
    jal audio_spu_set_common_attributes
    addiu $a0, $sp, 0x10
    lw $ra, 0x38($sp)
    addiu $sp, $sp, 0x40
    jr $ra
    nop
.end audio_spu_set_output_switch

.section .text
.balign 4
# exe/slus_004_22@0x8016BD2C..0x8016C0C0
.globl audio_spu_set_common_attributes
.ent audio_spu_set_common_attributes
audio_spu_set_common_attributes:
    addiu $sp, $sp, -0x10
    addu $a2, $zero, $zero
    lw $t1, 0($a0)
    nop
    sltiu $t2, $t1, 1
    bnez $t2, .Lscheduler_8016bd5c
    addu $t0, $zero, $zero
    andi $v0, $t1, 1
    beqz $v0, .Lscheduler_8016be0c
    andi $v0, $t1, 4
    beqz $v0, .Lscheduler_8016bdc0
    nop
.Lscheduler_8016bd5c:
    lh $v1, 8($a0)
    nop
    sltiu $v0, $v1, 8
    beqz $v0, .Lscheduler_8016bdc0
    sll $v0, $v1, 2
    lui $at, %hi(audio_spu_common_left_modes)
    addu $at, $at, $v0
    lw $v0, %lo(audio_spu_common_left_modes)($at)
    nop
    jr $v0
    nop
.Lscheduler_8016bd88:
    j .Lscheduler_8016bdc8
    ori $a1, $zero, 0x8000
.Lscheduler_8016bd90:
    j .Lscheduler_8016bdc8
    ori $a1, $zero, 0x9000
.Lscheduler_8016bd98:
    j .Lscheduler_8016bdc8
    ori $a1, $zero, 0xa000
.Lscheduler_8016bda0:
    j .Lscheduler_8016bdc8
    ori $a1, $zero, 0xb000
.Lscheduler_8016bda8:
    j .Lscheduler_8016bdc8
    ori $a1, $zero, 0xc000
.Lscheduler_8016bdb0:
    j .Lscheduler_8016bdc8
    ori $a1, $zero, 0xd000
.Lscheduler_8016bdb8:
    j .Lscheduler_8016bdc8
    ori $a1, $zero, 0xe000
.Lscheduler_8016bdc0:
    lhu $a2, 4($a0)
    addu $a1, $zero, $zero
.Lscheduler_8016bdc8:
    beqz $a1, .Lscheduler_8016bdfc
    andi $v0, $a2, 0x7fff
    lh $a3, 4($a0)
    nop
    slti $v0, $a3, 0x80
    bnez $v0, .Lscheduler_8016bdec
    addu $v1, $a3, $zero
    j .Lscheduler_8016bdf8
    addiu $a2, $zero, 0x7f
.Lscheduler_8016bdec:
    bgez $a3, .Lscheduler_8016bdf8
    addu $a2, $v1, $zero
    addu $a2, $zero, $zero
.Lscheduler_8016bdf8:
    andi $v0, $a2, 0x7fff
.Lscheduler_8016bdfc:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    or $v0, $v0, $a1
    sh $v0, 0x180($v1)
.Lscheduler_8016be0c:
    bnez $t2, .Lscheduler_8016be24
    andi $v0, $t1, 2
    beqz $v0, .Lscheduler_8016bed4
    andi $v0, $t1, 8
    beqz $v0, .Lscheduler_8016be88
    nop
.Lscheduler_8016be24:
    lh $v1, 0xa($a0)
    nop
    sltiu $v0, $v1, 8
    beqz $v0, .Lscheduler_8016be88
    sll $v0, $v1, 2
    lui $at, %hi(audio_spu_common_right_modes)
    addu $at, $at, $v0
    lw $v0, %lo(audio_spu_common_right_modes)($at)
    nop
    jr $v0
    nop
.Lscheduler_8016be50:
    j .Lscheduler_8016be90
    ori $a1, $zero, 0x8000
.Lscheduler_8016be58:
    j .Lscheduler_8016be90
    ori $a1, $zero, 0x9000
.Lscheduler_8016be60:
    j .Lscheduler_8016be90
    ori $a1, $zero, 0xa000
.Lscheduler_8016be68:
    j .Lscheduler_8016be90
    ori $a1, $zero, 0xb000
.Lscheduler_8016be70:
    j .Lscheduler_8016be90
    ori $a1, $zero, 0xc000
.Lscheduler_8016be78:
    j .Lscheduler_8016be90
    ori $a1, $zero, 0xd000
.Lscheduler_8016be80:
    j .Lscheduler_8016be90
    ori $a1, $zero, 0xe000
.Lscheduler_8016be88:
    lhu $t0, 6($a0)
    addu $a1, $zero, $zero
.Lscheduler_8016be90:
    beqz $a1, .Lscheduler_8016bec4
    andi $v0, $t0, 0x7fff
    lh $a2, 6($a0)
    nop
    slti $v0, $a2, 0x80
    bnez $v0, .Lscheduler_8016beb4
    addu $v1, $a2, $zero
    j .Lscheduler_8016bec0
    addiu $t0, $zero, 0x7f
.Lscheduler_8016beb4:
    bgez $a2, .Lscheduler_8016bec0
    addu $t0, $v1, $zero
    addu $t0, $zero, $zero
.Lscheduler_8016bec0:
    andi $v0, $t0, 0x7fff
.Lscheduler_8016bec4:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    or $v0, $v0, $a1
    sh $v0, 0x182($v1)
.Lscheduler_8016bed4:
    bnez $t2, .Lscheduler_8016bee4
    andi $v0, $t1, 0x40
    beqz $v0, .Lscheduler_8016bef8
    nop
.Lscheduler_8016bee4:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x10($a0)
    nop
    sh $v0, 0x1b0($v1)
.Lscheduler_8016bef8:
    bnez $t2, .Lscheduler_8016bf08
    andi $v0, $t1, 0x80
    beqz $v0, .Lscheduler_8016bf1c
    nop
.Lscheduler_8016bf08:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x12($a0)
    nop
    sh $v0, 0x1b2($v1)
.Lscheduler_8016bf1c:
    bnez $t2, .Lscheduler_8016bf2c
    andi $v0, $t1, 0x400
    beqz $v0, .Lscheduler_8016bf40
    nop
.Lscheduler_8016bf2c:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x1c($a0)
    nop
    sh $v0, 0x1b4($v1)
.Lscheduler_8016bf40:
    bnez $t2, .Lscheduler_8016bf50
    andi $v0, $t1, 0x800
    beqz $v0, .Lscheduler_8016bf64
    nop
.Lscheduler_8016bf50:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    lhu $v0, 0x1e($a0)
    nop
    sh $v0, 0x1b6($v1)
.Lscheduler_8016bf64:
    bnez $t2, .Lscheduler_8016bf74
    andi $v0, $t1, 0x100
    beqz $v0, .Lscheduler_8016bfb8
    nop
.Lscheduler_8016bf74:
    lw $v0, 0x14($a0)
    nop
    bnez $v0, .Lscheduler_8016bf9c
    nop
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    j .Lscheduler_8016bfb4
    andi $v1, $v1, 0xfffb
.Lscheduler_8016bf9c:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    nop
    ori $v1, $v1, 4
.Lscheduler_8016bfb4:
    sh $v1, 0x1aa($v0)
.Lscheduler_8016bfb8:
    bnez $t2, .Lscheduler_8016bfc8
    andi $v0, $t1, 0x200
    beqz $v0, .Lscheduler_8016c00c
    nop
.Lscheduler_8016bfc8:
    lw $v0, 0x18($a0)
    nop
    bnez $v0, .Lscheduler_8016bff0
    nop
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    j .Lscheduler_8016c008
    andi $v1, $v1, 0xfffe
.Lscheduler_8016bff0:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    nop
    ori $v1, $v1, 1
.Lscheduler_8016c008:
    sh $v1, 0x1aa($v0)
.Lscheduler_8016c00c:
    bnez $t2, .Lscheduler_8016c01c
    andi $v0, $t1, 0x1000
    beqz $v0, .Lscheduler_8016c060
    nop
.Lscheduler_8016c01c:
    lw $v0, 0x20($a0)
    nop
    bnez $v0, .Lscheduler_8016c044
    nop
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    j .Lscheduler_8016c05c
    andi $v1, $v1, 0xfff7
.Lscheduler_8016c044:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    nop
    ori $v1, $v1, 8
.Lscheduler_8016c05c:
    sh $v1, 0x1aa($v0)
.Lscheduler_8016c060:
    bnez $t2, .Lscheduler_8016c070
    andi $v0, $t1, 0x2000
    beqz $v0, .Lscheduler_8016c0b4
    nop
.Lscheduler_8016c070:
    lw $v0, 0x24($a0)
    nop
    bnez $v0, .Lscheduler_8016c098
    nop
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    j .Lscheduler_8016c0b0
    andi $v1, $v1, 0xfffd
.Lscheduler_8016c098:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v1, 0x1aa($v0)
    nop
    ori $v1, $v1, 2
.Lscheduler_8016c0b0:
    sh $v1, 0x1aa($v0)
.Lscheduler_8016c0b4:
    addiu $sp, $sp, 0x10
    jr $ra
    nop
.end audio_spu_set_common_attributes

.section .text
.balign 4
# exe/slus_004_22@0x8016C0C0..0x8016C110
.globl audio_spu_set_master_volume
.ent audio_spu_set_master_volume
audio_spu_set_master_volume:
    addiu $sp, $sp, -0x40
    addiu $v0, $zero, 3
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    sw $v0, 0x10($sp)
    sll $v0, $a0, 7
    addu $v0, $v0, $a0
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
    sh $v0, 0x14($sp)
    sll $v0, $a1, 7
    addu $v0, $v0, $a1
    addiu $a0, $sp, 0x10
    sw $ra, 0x38($sp)
    jal audio_spu_set_common_attributes
    sh $v0, 0x16($sp)
    lw $ra, 0x38($sp)
    addiu $sp, $sp, 0x40
    jr $ra
    nop
.end audio_spu_set_master_volume

.section .text
.balign 4
# exe/slus_004_22@0x8016C210..0x8016C478
.globl audio_sequence_register_callbacks
.ent audio_sequence_register_callbacks
audio_sequence_register_callbacks:
    addiu $sp, $sp, -0x20
    addiu $v0, $zero, 0x3e7
    sw $ra, 0x18($sp)
    sw $s1, 0x14($sp)
    sw $s0, 0x10($sp)
    addiu $v0, $v0, -1
.Lscheduler_8016c228:
    bgez $v0, .Lscheduler_8016c228
    addiu $v0, $v0, -1
    lui $s1, 0xf200
    ori $s1, $s1, 2
    lui $a1, %hi(audio_sequence_pitch_control_words + 0x8)
    addiu $a1, $a1, %lo(audio_sequence_pitch_control_words + 0x8)
    sb $zero, 0($a1)
    lui $v1, %hi(audio_sequence_tick_mode)
    lw $v1, %lo(audio_sequence_tick_mode)($v1)
    addiu $v0, $zero, 6
    lui $at, %hi(audio_sequence_pitch_control_words + 0xa)
    sb $v0, %lo(audio_sequence_pitch_control_words + 0xa)($at)
    addiu $v0, $zero, 2
    lui $at, %hi(audio_sequence_pitch_control_words + 0x9)
    sb $zero, %lo(audio_sequence_pitch_control_words + 0x9)($at)
    lui $at, %hi(audio_sequence_pitch_control_words + 0x4)
    sw $zero, %lo(audio_sequence_pitch_control_words + 0x4)($at)
    beq $v1, $v0, .Lscheduler_8016c39c
    addiu $s0, $zero, 0x44e8
    slti $v0, $v1, 3
    beqz $v0, .Lscheduler_8016c290
    nop
    beqz $v1, .Lscheduler_8016c2ac
    addiu $v0, $zero, 0x7f
    j .Lscheduler_8016c2ec
    nop
.Lscheduler_8016c290:
    addiu $v0, $zero, 3
    beq $v1, $v0, .Lscheduler_8016c2e4
    addiu $v0, $zero, 5
    beq $v1, $v0, .Lscheduler_8016c2bc
    nop
    j .Lscheduler_8016c2ec
    nop
.Lscheduler_8016c2ac:
    lui $at, %hi(audio_sequence_pitch_control_words + 0xa)
    sb $v0, %lo(audio_sequence_pitch_control_words + 0xa)($at)
    j .Lscheduler_8016c460
    nop
.Lscheduler_8016c2bc:
    lui $at, %hi(audio_sequence_pitch_control_words + 0xa)
    sb $zero, %lo(audio_sequence_pitch_control_words + 0xa)($at)
    bnez $a0, .Lscheduler_8016c2d4
    addiu $v0, $zero, 1
    j .Lscheduler_8016c39c
    sb $v0, 0($a1)
.Lscheduler_8016c2d4:
    lui $s1, 0xf200
    ori $s1, $s1, 3
    j .Lscheduler_8016c39c
    addiu $s0, $zero, 1
.Lscheduler_8016c2e4:
    j .Lscheduler_8016c39c
    ori $s0, $zero, 0x89d0
.Lscheduler_8016c2ec:
    lui $a1, %hi(audio_sequence_tick_flag)
    addiu $a1, $a1, %lo(audio_sequence_tick_flag)
    lw $v0, 0($a1)
    nop
    bnez $v0, .Lscheduler_8016c460
    nop
    lw $a0, -4($a1)
    nop
    slti $v0, $a0, 0x46
    beqz $v0, .Lscheduler_8016c360
    lui $v1, 0x20
    ori $v1, $v1, 0x4cc0
    div $zero, $v1, $a0
    bnez $a0, .Lscheduler_8016c32c
    nop
    break 7
.Lscheduler_8016c32c:
    addiu $at, $zero, -1
    bne $a0, $at, .Lscheduler_8016c344
    lui $at, 0x8000
    bne $v1, $at, .Lscheduler_8016c344
    nop
    break 6
.Lscheduler_8016c344:
    mflo $v1
    lbu $v0, 0xd($a1)
    nop
    addiu $v0, $v0, 1
    sb $v0, 0xd($a1)
    j .Lscheduler_8016c39c
    addu $s0, $v1, $zero
.Lscheduler_8016c360:
    lui $v0, 0x40
    ori $v0, $v0, 0x9980
    div $zero, $v0, $a0
    bnez $a0, .Lscheduler_8016c378
    nop
    break 7
.Lscheduler_8016c378:
    addiu $at, $zero, -1
    bne $a0, $at, .Lscheduler_8016c390
    lui $at, 0x8000
    bne $v0, $at, .Lscheduler_8016c390
    nop
    break 6
.Lscheduler_8016c390:
    mflo $v0
    nop
    addu $s0, $v0, $zero
.Lscheduler_8016c39c:
    lui $v0, %hi(audio_sequence_pitch_control_words + 0x8)
    lbu $v0, %lo(audio_sequence_pitch_control_words + 0x8)($v0)
    nop
    beqz $v0, .Lscheduler_8016c3d0
    nop
    jal audio_spu_service_enter_critical
    nop
    lui $a0, %hi(audio_sequence_pitch_control_words)
    lw $a0, %lo(audio_sequence_pitch_control_words)($a0)
    jal audio_callbacks_vblank_slot4
    nop
    j .Lscheduler_8016c458
    nop
.Lscheduler_8016c3d0:
    jal audio_spu_service_enter_critical
    nop
    jal audio_counter_reset
    addu $a0, $s1, $zero
    addu $a0, $s1, $zero
    andi $a1, $s0, 0xffff
    jal audio_counter_set
    addiu $a2, $zero, 0x1000
    lui $a0, %hi(audio_sequence_pitch_control_words + 0xa)
    lbu $a0, %lo(audio_sequence_pitch_control_words + 0xa)($a0)
    nop
    bnez $a0, .Lscheduler_8016c430
    nop
    addu $a0, $zero, $zero
    jal audio_callbacks_interrupt
    addu $a1, $zero, $zero
    lui $a0, %hi(audio_sequence_pitch_control_words + 0xa)
    lbu $a0, %lo(audio_sequence_pitch_control_words + 0xa)($a0)
    lui $a1, %hi(audio_sequence_dispatch_callbacks)
    addiu $a1, $a1, %lo(audio_sequence_dispatch_callbacks)
    lui $at, %hi(audio_sequence_pitch_control_words + 0x4)
    sw $v0, %lo(audio_sequence_pitch_control_words + 0x4)($at)
    j .Lscheduler_8016c450
    nop
.Lscheduler_8016c430:
    lui $v0, %hi(audio_sequence_pitch_control_words + 0x9)
    lbu $v0, %lo(audio_sequence_pitch_control_words + 0x9)($v0)
    lui $a1, %hi(audio_sequence_dispatch_half_rate)
    addiu $a1, $a1, %lo(audio_sequence_dispatch_half_rate)
    bnez $v0, .Lscheduler_8016c450
    nop
    lui $a1, %hi(audio_sequence_pitch_control_words)
    lw $a1, %lo(audio_sequence_pitch_control_words)($a1)
.Lscheduler_8016c450:
    jal audio_callbacks_interrupt
    nop
.Lscheduler_8016c458:
    jal audio_spu_service_exit_critical
    nop
.Lscheduler_8016c460:
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_register_callbacks

.section .text
.balign 4
# exe/slus_004_22@0x8016C478..0x8016C498
.globl audio_sequence_start_callbacks_alternate
.ent audio_sequence_start_callbacks_alternate
audio_sequence_start_callbacks_alternate:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_sequence_register_callbacks
    addiu $a0, $zero, 1
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_start_callbacks_alternate

.section .text
.balign 4
# exe/slus_004_22@0x8016C498..0x8016C4B8
.globl audio_sequence_start_callbacks
.ent audio_sequence_start_callbacks
audio_sequence_start_callbacks:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_sequence_register_callbacks
    addu $a0, $zero, $zero
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_start_callbacks

.section .text
.balign 4
# exe/slus_004_22@0x8016C4B8..0x8016C4F8
.globl audio_sequence_dispatch_callbacks
.ent audio_sequence_dispatch_callbacks
audio_sequence_dispatch_callbacks:
    lui $v0, %hi(audio_sequence_pitch_control_words + 0x4)
    lw $v0, %lo(audio_sequence_pitch_control_words + 0x4)($v0)
    addiu $sp, $sp, -0x18
    beqz $v0, .Lscheduler_8016c4d4
    sw $ra, 0x10($sp)
    jalr $v0
    nop
.Lscheduler_8016c4d4:
    lui $v0, %hi(audio_sequence_pitch_control_words)
    lw $v0, %lo(audio_sequence_pitch_control_words)($v0)
    nop
    jalr $v0
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_dispatch_callbacks

.section .text
.balign 4
# exe/slus_004_22@0x8016C4F8..0x8016C548
.globl audio_sequence_dispatch_half_rate
.ent audio_sequence_dispatch_half_rate
audio_sequence_dispatch_half_rate:
    lui $v0, %hi(audio_sequence_pitch_control_words + 0xc)
    lw $v0, %lo(audio_sequence_pitch_control_words + 0xc)($v0)
    addiu $sp, $sp, -0x18
    bnez $v0, .Lscheduler_8016c520
    sw $ra, 0x10($sp)
    addiu $v0, $zero, 1
    lui $at, %hi(audio_sequence_pitch_control_words + 0xc)
    sw $v0, %lo(audio_sequence_pitch_control_words + 0xc)($at)
    j .Lscheduler_8016c538
    nop
.Lscheduler_8016c520:
    lui $v0, %hi(audio_sequence_pitch_control_words)
    lw $v0, %lo(audio_sequence_pitch_control_words)($v0)
    lui $at, %hi(audio_sequence_pitch_control_words + 0xc)
    sw $zero, %lo(audio_sequence_pitch_control_words + 0xc)($at)
    jalr $v0
    nop
.Lscheduler_8016c538:
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_dispatch_half_rate

.section .text.audio_sequence_scheduler,"ax"
.balign 4
# exe/slus_004_22@0x8016C548..0x8016C7BC
.globl audio_sequence_scheduler
.ent audio_sequence_scheduler
audio_sequence_scheduler:
    addiu $sp, $sp, -0x48
    lui $v0, %hi(audio_sequence_voice_globals + 0x6f8)
    lw $v0, %lo(audio_sequence_voice_globals + 0x6f8)($v0)
    addiu $v1, $zero, 1
    sw $ra, 0x44($sp)
    sw $fp, 0x40($sp)
    sw $s7, 0x3c($sp)
    sw $s6, 0x38($sp)
    sw $s5, 0x34($sp)
    sw $s4, 0x30($sp)
    sw $s3, 0x2c($sp)
    sw $s2, 0x28($sp)
    sw $s1, 0x24($sp)
    beq $v0, $v1, .Lscheduler_8016c788
    sw $s0, 0x20($sp)
    lui $at, %hi(audio_sequence_voice_globals + 0x6f8)
    sw $v1, %lo(audio_sequence_voice_globals + 0x6f8)($at)
    jal audio_sequence_flush_voices
    addu $s7, $zero, $zero
    lui $v0, %hi(audio_sequence_handle_count)
    lh $v0, %lo(audio_sequence_handle_count)($v0)
    nop
    blez $v0, .Lscheduler_8016c780
    nop
    lui $fp, %hi(audio_sequence_handles)
    addiu $fp, $fp, %lo(audio_sequence_handles)
.Lscheduler_8016c5b0:
    addiu $v0, $zero, 1
    lui $v1, %hi(audio_sequence_allocation_mask)
    lw $v1, %lo(audio_sequence_allocation_mask)($v1)
    sllv $v0, $v0, $s7
    and $v1, $v1, $v0
    beqz $v1, .Lscheduler_8016c768
    nop
    lui $v0, %hi(audio_sequence_count)
    lh $v0, %lo(audio_sequence_count)($v0)
    nop
    blez $v0, .Lscheduler_8016c768
    addu $s6, $zero, $zero
    addu $s2, $fp, $zero
    sll $s5, $s7, 0x10
    sra $s4, $s5, 0x10
    addu $s3, $zero, $zero
    addu $s0, $zero, $zero
.Lscheduler_8016c5f4:
    lw $v0, 0($s2)
    nop
    addu $v0, $s0, $v0
    lw $v0, 0x90($v0)
    nop
    andi $v0, $v0, 1
    beqz $v0, .Lscheduler_8016c6c0
    addu $a0, $s4, $zero
    sra $s1, $s3, 0x10
    jal audio_sequence_tick_record
    addu $a1, $s1, $zero
    lw $v0, 0($s2)
    nop
    addu $v0, $s0, $v0
    lw $v0, 0x90($v0)
    nop
    andi $v0, $v0, 0x10
    beqz $v0, .Lscheduler_8016c648
    addu $a0, $s4, $zero
    jal audio_sequence_increase_gain
    addu $a1, $s1, $zero
.Lscheduler_8016c648:
    lw $v0, 0($s2)
    nop
    addu $v0, $s0, $v0
    lw $v0, 0x90($v0)
    nop
    andi $v0, $v0, 0x20
    beqz $v0, .Lscheduler_8016c670
    addu $a0, $s4, $zero
    jal audio_sequence_decrease_gain
    addu $a1, $s1, $zero
.Lscheduler_8016c670:
    lw $v0, 0($s2)
    nop
    addu $v0, $s0, $v0
    lw $v0, 0x90($v0)
    nop
    andi $v0, $v0, 0x40
    beqz $v0, .Lscheduler_8016c698
    addu $a0, $s4, $zero
    jal audio_sequence_ramp_tempo
    addu $a1, $s1, $zero
.Lscheduler_8016c698:
    lw $v0, 0($s2)
    nop
    addu $v0, $s0, $v0
    lw $v0, 0x90($v0)
    nop
    andi $v0, $v0, 0x80
    beqz $v0, .Lscheduler_8016c6c0
    addu $a0, $s4, $zero
    jal audio_sequence_ramp_tempo
    addu $a1, $s1, $zero
.Lscheduler_8016c6c0:
    lw $v0, 0($s2)
    nop
    addu $v0, $s0, $v0
    lw $v0, 0x90($v0)
    nop
    andi $v0, $v0, 2
    beqz $v0, .Lscheduler_8016c6e8
    sra $a0, $s5, 0x10
    jal audio_sequence_stop_request
    sra $a1, $s3, 0x10
.Lscheduler_8016c6e8:
    lw $v0, 0($s2)
    nop
    addu $v0, $s0, $v0
    lw $v0, 0x90($v0)
    nop
    andi $v0, $v0, 8
    beqz $v0, .Lscheduler_8016c710
    sra $a0, $s5, 0x10
    jal audio_sequence_resume_record
    sra $a1, $s3, 0x10
.Lscheduler_8016c710:
    lw $v0, 0($s2)
    nop
    addu $v0, $s0, $v0
    lw $v0, 0x90($v0)
    nop
    andi $v0, $v0, 4
    beqz $v0, .Lscheduler_8016c748
    sra $a0, $s5, 0x10
    jal audio_sequence_stop_record
    sra $a1, $s3, 0x10
    lw $v0, 0($s2)
    nop
    addu $v0, $s0, $v0
    sw $zero, 0x90($v0)
.Lscheduler_8016c748:
    lui $v0, 1
    addu $s3, $s3, $v0
    lui $v0, %hi(audio_sequence_count)
    lh $v0, %lo(audio_sequence_count)($v0)
    addiu $s6, $s6, 1
    slt $v0, $s6, $v0
    bnez $v0, .Lscheduler_8016c5f4
    addiu $s0, $s0, 0xac
.Lscheduler_8016c768:
    lui $v0, %hi(audio_sequence_handle_count)
    lh $v0, %lo(audio_sequence_handle_count)($v0)
    addiu $s7, $s7, 1
    slt $v0, $s7, $v0
    bnez $v0, .Lscheduler_8016c5b0
    addiu $fp, $fp, 4
.Lscheduler_8016c780:
    lui $at, %hi(audio_sequence_voice_globals + 0x6f8)
    sw $zero, %lo(audio_sequence_voice_globals + 0x6f8)($at)
.Lscheduler_8016c788:
    lw $ra, 0x44($sp)
    lw $fp, 0x40($sp)
    lw $s7, 0x3c($sp)
    lw $s6, 0x38($sp)
    lw $s5, 0x34($sp)
    lw $s4, 0x30($sp)
    lw $s3, 0x2c($sp)
    lw $s2, 0x28($sp)
    lw $s1, 0x24($sp)
    lw $s0, 0x20($sp)
    addiu $sp, $sp, 0x48
    jr $ra
    nop
.end audio_sequence_scheduler

.section .text
.balign 4
# exe/slus_004_22@0x8016C7BC..0x8016CA9C
.globl audio_sequence_increase_gain
.ent audio_sequence_increase_gain
audio_sequence_increase_gain:
    addiu $sp, $sp, -0x40
    addu $a3, $a0, $zero
    sll $v0, $a3, 0x10
    lui $v1, %hi(audio_sequence_handles)
    addiu $v1, $v1, %lo(audio_sequence_handles)
    sra $v0, $v0, 0xe
    sw $s2, 0x30($sp)
    addu $s2, $v0, $v1
    sll $v1, $a1, 0x10
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sw $s1, 0x2c($sp)
    sll $s1, $v0, 2
    sw $s4, 0x38($sp)
    addu $s4, $a3, $zero
    sw $ra, 0x3c($sp)
    sw $s3, 0x34($sp)
    sw $s0, 0x28($sp)
    lw $v1, 0($s2)
    addu $s3, $a1, $zero
    addu $s0, $v1, $s1
    lw $v0, 0x98($s0)
    lh $a2, 0x42($s0)
    addiu $v0, $v0, -1
    addu $v1, $a2, $zero
    blez $a2, .Lscheduler_8016c8cc
    sw $v0, 0x98($s0)
    divu $zero, $v0, $a2
    bnez $a2, .Lscheduler_8016c84c
    nop
    break 7
.Lscheduler_8016c84c:
    mfhi $v0
    nop
    bnez $v0, .Lscheduler_8016ca5c
    nop
    lhu $v0, 0x40($s0)
    nop
    addiu $v0, $v0, -1
    sh $v0, 0x40($s0)
    sll $v0, $v0, 0x10
    bltz $v0, .Lscheduler_8016c9b4
    sll $v0, $a1, 8
    or $v0, $a3, $v0
    sll $v0, $v0, 0x10
    sra $s1, $v0, 0x10
    addu $a0, $s1, $zero
    addiu $a1, $sp, 0x10
    jal audio_sequence_get_gain
    addiu $a2, $sp, 0x12
    lhu $a1, 0x10($sp)
    lh $v1, 0x40($s0)
    andi $v0, $a1, 0xffff
    addiu $a0, $v0, 1
    addu $v0, $v0, $v1
    slt $v0, $v0, $a0
    bnez $v0, .Lscheduler_8016c9f0
    addu $a0, $s1, $zero
    addiu $a1, $a1, 1
    andi $a1, $a1, 0xffff
    lhu $a2, 0x12($sp)
    addiu $a3, $zero, 1
    j .Lscheduler_8016c9a4
    addiu $a2, $a2, 1
.Lscheduler_8016c8cc:
    bgez $a2, .Lscheduler_8016ca60
    sll $a0, $s3, 8
    lhu $v0, 0x40($s0)
    nop
    addu $v0, $v0, $v1
    sh $v0, 0x40($s0)
    sll $v0, $v0, 0x10
    bltz $v0, .Lscheduler_8016c9b4
    sll $v0, $a1, 8
    or $v0, $a3, $v0
    sll $v0, $v0, 0x10
    sra $s1, $v0, 0x10
    addu $a0, $s1, $zero
    addiu $a1, $sp, 0x10
    jal audio_sequence_get_gain
    addiu $a2, $sp, 0x12
    lhu $v0, 0x10($sp)
    lh $v1, 0x42($s0)
    nop
    subu $v0, $v0, $v1
    slti $v0, $v0, 0x7f
    bnez $v0, .Lscheduler_8016c950
    nop
    lhu $v0, 0x12($sp)
    nop
    subu $v0, $v0, $v1
    slti $v0, $v0, 0x7f
    bnez $v0, .Lscheduler_8016c950
    addu $a0, $s1, $zero
    addiu $a1, $zero, 0x7f
    addiu $a2, $zero, 0x7f
    jal audio_sequence_set_gain
    addiu $a3, $zero, 1
.Lscheduler_8016c950:
    lw $v1, 0x94($s0)
    lw $v0, 0x98($s0)
    lh $a0, 0x42($s0)
    subu $v1, $v1, $v0
    negu $v0, $a0
    mult $v1, $v0
    lh $v0, 0x3e($s0)
    mflo $t0
    sltu $v0, $t0, $v0
    beqz $v0, .Lscheduler_8016c9f0
    addu $v1, $a0, $zero
    sll $a0, $s3, 8
    or $a0, $s4, $a0
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    addiu $a3, $zero, 1
    lhu $a1, 0x10($sp)
    lhu $a2, 0x12($sp)
    subu $a1, $a1, $v1
    andi $a1, $a1, 0xffff
    subu $a2, $a2, $v1
.Lscheduler_8016c9a4:
    jal audio_sequence_set_gain
    andi $a2, $a2, 0xffff
    j .Lscheduler_8016c9f0
    nop
.Lscheduler_8016c9b4:
    sll $a0, $a1, 8
    or $a0, $a3, $a0
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    addiu $a1, $zero, 0x7f
    addiu $a2, $zero, 0x7f
    jal audio_sequence_set_gain
    addiu $a3, $zero, 1
    lw $v1, 0($s2)
    nop
    addu $v1, $s1, $v1
    lw $v0, 0x90($v1)
    addiu $a0, $zero, -0x11
    and $v0, $v0, $a0
    sw $v0, 0x90($v1)
.Lscheduler_8016c9f0:
    lw $v0, 0x98($s0)
    nop
    beqz $v0, .Lscheduler_8016ca14
    sll $a0, $s4, 0x10
    lh $v0, 0x40($s0)
    nop
    bnez $v0, .Lscheduler_8016ca60
    sll $a0, $s3, 8
    sll $a0, $s4, 0x10
.Lscheduler_8016ca14:
    sra $a0, $a0, 0xe
    sll $v1, $s3, 0x10
    sra $v1, $v1, 0x10
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
    addu $v0, $v0, $v1
    lw $v1, 0x90($v0)
    addiu $a0, $zero, -0x11
    and $v1, $v1, $a0
    sw $v1, 0x90($v0)
.Lscheduler_8016ca5c:
    sll $a0, $s3, 8
.Lscheduler_8016ca60:
    or $a0, $s4, $a0
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    addiu $a1, $s0, 0x78
    jal audio_sequence_get_gain
    addiu $a2, $s0, 0x7a
    lw $ra, 0x3c($sp)
    lw $s4, 0x38($sp)
    lw $s3, 0x34($sp)
    lw $s2, 0x30($sp)
    lw $s1, 0x2c($sp)
    lw $s0, 0x28($sp)
    addiu $sp, $sp, 0x40
    jr $ra
    nop
.end audio_sequence_increase_gain

.section .text
.balign 4
# exe/slus_004_22@0x8016CA9C..0x8016CD3C
.globl audio_sequence_decrease_gain
.ent audio_sequence_decrease_gain
audio_sequence_decrease_gain:
    addiu $sp, $sp, -0x40
    sll $v0, $a0, 0x10
    lui $v1, %hi(audio_sequence_handles)
    addiu $v1, $v1, %lo(audio_sequence_handles)
    sra $v0, $v0, 0xe
    addu $t0, $v0, $v1
    sll $v1, $a1, 0x10
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $a3, $v0, 2
    sw $s3, 0x34($sp)
    addu $s3, $a0, $zero
    sw $ra, 0x38($sp)
    sw $s2, 0x30($sp)
    sw $s1, 0x2c($sp)
    sw $s0, 0x28($sp)
    lw $v1, 0($t0)
    addu $s2, $a1, $zero
    addu $s0, $v1, $a3
    lw $v0, 0x98($s0)
    lh $a2, 0x42($s0)
    addiu $v0, $v0, -1
    addu $v1, $a2, $zero
    blez $a2, .Lscheduler_8016cbb4
    sw $v0, 0x98($s0)
    divu $zero, $v0, $a2
    bnez $a2, .Lscheduler_8016cb24
    nop
    break 7
.Lscheduler_8016cb24:
    mfhi $v0
    nop
    bnez $v0, .Lscheduler_8016cd00
    nop
    lhu $v0, 0x40($s0)
    nop
    addiu $v0, $v0, -1
    sh $v0, 0x40($s0)
    sll $v0, $v0, 0x10
    blez $v0, .Lscheduler_8016cc78
    sll $v0, $a1, 8
    or $v0, $a0, $v0
    sll $v0, $v0, 0x10
    sra $s1, $v0, 0x10
    addu $a0, $s1, $zero
    addiu $a1, $sp, 0x10
    jal audio_sequence_get_gain
    addiu $a2, $sp, 0x12
    lhu $a1, 0x10($sp)
    lh $v1, 0x40($s0)
    andi $a0, $a1, 0xffff
    subu $v0, $a0, $v1
    blez $v0, .Lscheduler_8016cc50
    nop
    lhu $a2, 0x12($sp)
    nop
    subu $v0, $a2, $v1
    blez $v0, .Lscheduler_8016cc50
    addiu $v0, $zero, 1
    beq $a0, $v0, .Lscheduler_8016cc50
    addu $a0, $s1, $zero
    addiu $a1, $a1, -1
    andi $a1, $a1, 0xffff
    addiu $a2, $a2, -1
    j .Lscheduler_8016cc68
    andi $a2, $a2, 0xffff
.Lscheduler_8016cbb4:
    lhu $v0, 0x40($s0)
    nop
    addu $v0, $v0, $v1
    sh $v0, 0x40($s0)
    sll $v0, $v0, 0x10
    blez $v0, .Lscheduler_8016cc78
    sll $v0, $a1, 8
    or $v0, $a0, $v0
    sll $v0, $v0, 0x10
    sra $s1, $v0, 0x10
    addu $a0, $s1, $zero
    addiu $a1, $sp, 0x10
    jal audio_sequence_get_gain
    addiu $a2, $sp, 0x12
    lw $v0, 0x94($s0)
    lw $v1, 0x98($s0)
    lh $a0, 0x42($s0)
    subu $v0, $v0, $v1
    negu $a2, $a0
    mult $v0, $a2
    lh $v0, 0x3e($s0)
    mflo $t1
    sltu $v0, $v0, $t1
    bnez $v0, .Lscheduler_8016cc50
    addu $v1, $a0, $zero
    lhu $a1, 0x10($sp)
    nop
    slt $v0, $a2, $a1
    beqz $v0, .Lscheduler_8016cc50
    addu $a0, $s1, $zero
    addu $a1, $a1, $v1
    andi $a1, $a1, 0xffff
    lhu $a2, 0x12($sp)
    addiu $a3, $zero, 1
    addu $a2, $a2, $v1
    jal audio_sequence_set_gain
    andi $a2, $a2, 0xffff
    j .Lscheduler_8016cc94
    nop
.Lscheduler_8016cc50:
    sll $a0, $s2, 8
    or $a0, $s3, $a0
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    addiu $a1, $zero, 1
    addiu $a2, $zero, 1
.Lscheduler_8016cc68:
    jal audio_sequence_set_gain
    addiu $a3, $zero, 1
    j .Lscheduler_8016cc94
    nop
.Lscheduler_8016cc78:
    lw $v0, 0($t0)
    nop
    addu $v0, $a3, $v0
    lw $v1, 0x90($v0)
    addiu $a0, $zero, -0x21
    and $v1, $v1, $a0
    sw $v1, 0x90($v0)
.Lscheduler_8016cc94:
    lw $v0, 0x98($s0)
    nop
    beqz $v0, .Lscheduler_8016ccb8
    sll $a0, $s3, 0x10
    lh $v0, 0x40($s0)
    nop
    bnez $v0, .Lscheduler_8016cd04
    sll $a0, $s2, 8
    sll $a0, $s3, 0x10
.Lscheduler_8016ccb8:
    sra $a0, $a0, 0xe
    sll $v1, $s2, 0x10
    sra $v1, $v1, 0x10
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
    addu $v0, $v0, $v1
    lw $v1, 0x90($v0)
    addiu $a0, $zero, -0x21
    and $v1, $v1, $a0
    sw $v1, 0x90($v0)
.Lscheduler_8016cd00:
    sll $a0, $s2, 8
.Lscheduler_8016cd04:
    or $a0, $s3, $a0
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    addiu $a1, $s0, 0x78
    jal audio_sequence_get_gain
    addiu $a2, $s0, 0x7a
    lw $ra, 0x38($sp)
    lw $s3, 0x34($sp)
    lw $s2, 0x30($sp)
    lw $s1, 0x2c($sp)
    lw $s0, 0x28($sp)
    addiu $sp, $sp, 0x40
    jr $ra
    nop
.end audio_sequence_decrease_gain

.section .text
.balign 4
# exe/slus_004_22@0x8016CD3C..0x8016CDE0
.globl audio_sequence_stop_request
.ent audio_sequence_stop_request
audio_sequence_stop_request:
    addiu $sp, $sp, -0x20
    sw $s2, 0x18($sp)
    sll $s2, $a0, 0x10
    lui $v0, %hi(audio_sequence_handles)
    addiu $v0, $v0, %lo(audio_sequence_handles)
    sra $s2, $s2, 0xe
    addu $s2, $s2, $v0
    sll $v0, $a1, 0x10
    sra $v0, $v0, 0x10
    sw $s0, 0x10($sp)
    sll $s0, $v0, 1
    addu $s0, $s0, $v0
    sll $s0, $s0, 2
    subu $s0, $s0, $v0
    sll $s0, $s0, 2
    subu $s0, $s0, $v0
    sll $s0, $s0, 2
    sll $a1, $a1, 8
    or $a0, $a0, $a1
    sll $a0, $a0, 0x10
    sw $ra, 0x1c($sp)
    sw $s1, 0x14($sp)
    lw $s1, 0($s2)
    sra $a0, $a0, 0x10
    jal audio_sequence_release_owner_voices
    addu $s1, $s1, $s0
    sb $zero, 0x2b($s1)
    lw $v0, 0($s2)
    nop
    addu $s0, $s0, $v0
    lw $v0, 0x90($s0)
    addiu $v1, $zero, -3
    and $v0, $v0, $v1
    sw $v0, 0x90($s0)
    lw $ra, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_stop_request

.section .text
.balign 4
# exe/slus_004_22@0x8016CDE0..0x8016CE0C
.globl audio_sequence_tick_record
.ent audio_sequence_tick_record
audio_sequence_tick_record:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    sll $a0, $a0, 0x10
    sll $a1, $a1, 0x10
    sra $a0, $a0, 0x10
    jal audio_sequence_consume_events
    sra $a1, $a1, 0x10
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_tick_record

.section .text
.balign 4
# exe/slus_004_22@0x8016CE0C..0x8016CF1C
.globl audio_sequence_consume_events
.ent audio_sequence_consume_events
audio_sequence_consume_events:
    addiu $sp, $sp, -0x38
    sll $a3, $a0, 0x10
    sra $a0, $a3, 0xe
    sll $a1, $a1, 0x10
    sra $v1, $a1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sw $ra, 0x30($sp)
    sw $s3, 0x2c($sp)
    sw $s2, 0x28($sp)
    sw $s1, 0x24($sp)
    sw $s0, 0x20($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a0
    lw $v1, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $s1, $v1, $v0
    lh $v0, 0x70($s1)
    lw $v1, 0x88($s1)
    nop
    subu $a0, $v1, $v0
    blez $a0, .Lscheduler_8016ceb4
    addu $a2, $v0, $zero
    lh $v1, 0x6e($s1)
    nop
    blez $v1, .Lscheduler_8016ce94
    addu $v0, $v1, $zero
    addiu $v0, $v0, -1
    j .Lscheduler_8016cefc
    sh $v0, 0x6e($s1)
.Lscheduler_8016ce94:
    bnez $v1, .Lscheduler_8016ceac
    nop
    lw $v0, 0x88($s1)
    sh $a2, 0x6e($s1)
    j .Lscheduler_8016cef8
    addiu $v0, $v0, -1
.Lscheduler_8016ceac:
    j .Lscheduler_8016cefc
    sw $a0, 0x88($s1)
.Lscheduler_8016ceb4:
    slt $v0, $v0, $v1
    bnez $v0, .Lscheduler_8016cefc
    addu $s0, $v1, $zero
    addu $s3, $a3, $zero
    addu $s2, $a1, $zero
    sra $a0, $s3, 0x10
.Lscheduler_8016cecc:
    jal audio_sequence_decode_event
    sra $a1, $s2, 0x10
    lw $v0, 0x88($s1)
    nop
    beqz $v0, .Lscheduler_8016cecc
    sra $a0, $s3, 0x10
    lh $v1, 0x70($s1)
    addu $s0, $s0, $v0
    slt $v0, $s0, $v1
    bnez $v0, .Lscheduler_8016cecc
    subu $v0, $s0, $v1
.Lscheduler_8016cef8:
    sw $v0, 0x88($s1)
.Lscheduler_8016cefc:
    lw $ra, 0x30($sp)
    lw $s3, 0x2c($sp)
    lw $s2, 0x28($sp)
    lw $s1, 0x24($sp)
    lw $s0, 0x20($sp)
    addiu $sp, $sp, 0x38
    jr $ra
    nop
.end audio_sequence_consume_events

.section .text
.balign 4
# exe/slus_004_22@0x8016CF1C..0x8016D0E0
.globl audio_sequence_end_event
.ent audio_sequence_end_event
audio_sequence_end_event:
    addiu $sp, $sp, -0x20
    sw $s2, 0x18($sp)
    addu $s2, $a0, $zero
    sw $s1, 0x14($sp)
    addu $s1, $a1, $zero
    sll $v0, $s2, 0x10
    lui $v1, %hi(audio_sequence_handles)
    addiu $v1, $v1, %lo(audio_sequence_handles)
    sra $v0, $v0, 0xe
    addu $a2, $v0, $v1
    sll $v1, $s1, 0x10
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sw $ra, 0x1c($sp)
    sw $s0, 0x10($sp)
    lw $v1, 0($a2)
    sll $a1, $v0, 2
    addu $s0, $v1, $a1
    lhu $v0, 0x48($s0)
    lh $v1, 0x46($s0)
    addiu $v0, $v0, 1
    bnez $v1, .Lscheduler_8016cfa4
    sh $v0, 0x48($s0)
    lw $v0, 8($s0)
    sw $zero, 0x80($s0)
    sb $zero, 0x27($s0)
    sw $zero, 0x88($s0)
    j .Lscheduler_8016d0c4
    sw $v0, 4($s0)
.Lscheduler_8016cfa4:
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slt $v0, $v0, $v1
    beqz $v0, .Lscheduler_8016cfd8
    addiu $a0, $zero, -2
    lw $v0, 8($s0)
    lw $v1, 8($s0)
    sw $zero, 0x80($s0)
    sb $zero, 0x27($s0)
    sw $zero, 0x88($s0)
    sw $v0, 4($s0)
    j .Lscheduler_8016d0c4
    sw $v1, 0xc($s0)
.Lscheduler_8016cfd8:
    lw $v1, 0($a2)
    nop
    addu $v1, $a1, $v1
    lw $v0, 0x90($v1)
    nop
    and $v0, $v0, $a0
    sw $v0, 0x90($v1)
    lw $v1, 0($a2)
    nop
    addu $v1, $a1, $v1
    lw $v0, 0x90($v1)
    addiu $a0, $zero, -9
    and $v0, $v0, $a0
    sw $v0, 0x90($v1)
    lw $v1, 0($a2)
    nop
    addu $v1, $a1, $v1
    lw $v0, 0x90($v1)
    addiu $a0, $zero, -3
    and $v0, $v0, $a0
    sw $v0, 0x90($v1)
    lw $v1, 0($a2)
    nop
    addu $v1, $a1, $v1
    lw $v0, 0x90($v1)
    nop
    ori $v0, $v0, 0x200
    sw $v0, 0x90($v1)
    lw $v1, 0($a2)
    nop
    addu $v1, $a1, $v1
    lw $v0, 0x90($v1)
    nop
    ori $v0, $v0, 4
    sw $v0, 0x90($v1)
    lw $v0, 8($s0)
    lbu $v1, 0x3c($s0)
    sb $zero, 0x2b($s0)
    sw $v0, 0xc($s0)
    addiu $v0, $zero, 0xff
    beq $v1, $v0, .Lscheduler_8016d0a4
    nop
    lbu $a0, 0x3c($s0)
    lbu $a1, 0($s0)
    jal audio_sequence_start_linked_record
    sb $zero, 0x2b($s0)
    sll $a0, $s1, 8
    or $a0, $s2, $a0
    sll $a0, $a0, 0x10
    jal audio_sequence_release_owner_voices
    sra $a0, $a0, 0x10
.Lscheduler_8016d0a4:
    sll $a0, $s1, 8
    or $a0, $s2, $a0
    sll $a0, $a0, 0x10
    jal audio_sequence_release_owner_voices
    sra $a0, $a0, 0x10
    lh $v0, 0x70($s0)
    nop
    sw $v0, 0x88($s0)
.Lscheduler_8016d0c4:
    lw $ra, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_end_event

.section .text
.balign 4
# exe/slus_004_22@0x8016D0E0..0x8016D3C4
.globl audio_sequence_decode_event
.ent audio_sequence_decode_event
audio_sequence_decode_event:
    addiu $sp, $sp, -0x28
    sll $a0, $a0, 0x10
    sw $s2, 0x18($sp)
    sra $s2, $a0, 0x10
    sll $v1, $s2, 2
    sll $a1, $a1, 0x10
    sw $s3, 0x1c($sp)
    sra $s3, $a1, 0x10
    sll $v0, $s3, 1
    addu $v0, $v0, $s3
    sll $v0, $v0, 2
    subu $v0, $v0, $s3
    sll $v0, $v0, 2
    subu $v0, $v0, $s3
    sw $ra, 0x24($sp)
    sw $s4, 0x20($sp)
    sw $s1, 0x14($sp)
    sw $s0, 0x10($sp)
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $v1
    lw $v1, %lo(audio_sequence_handles)($at)
    sll $v0, $v0, 2
    addu $s1, $v1, $v0
    lw $a3, 4($s1)
    nop
    addiu $v0, $a3, 1
    sw $v0, 4($s1)
    lbu $s0, 0($a3)
    nop
    andi $v0, $s0, 0x80
    beqz $v0, .Lscheduler_8016d290
    andi $a0, $s0, 0xf
    andi $a2, $s0, 0xf0
    addiu $v0, $zero, 0xc0
    beq $a2, $v0, .Lscheduler_8016d220
    sb $a0, 0x12($s1)
    slti $v0, $a2, 0xc1
    beqz $v0, .Lscheduler_8016d194
    addiu $v0, $zero, 0x90
    beq $a2, $v0, .Lscheduler_8016d1b0
    addiu $v0, $zero, 0xb0
    beq $a2, $v0, .Lscheduler_8016d1fc
    addiu $v0, $zero, 0xb0
    j .Lscheduler_8016d3a0
    nop
.Lscheduler_8016d194:
    addiu $v0, $zero, 0xe0
    beq $a2, $v0, .Lscheduler_8016d248
    addiu $v0, $zero, 0xf0
    beq $a2, $v0, .Lscheduler_8016d264
    addiu $v0, $zero, 0xff
    j .Lscheduler_8016d3a0
    nop
.Lscheduler_8016d1b0:
    addu $a0, $s2, $zero
    lw $v1, 4($s1)
    addiu $v0, $zero, 0x90
    sb $v0, 0x11($s1)
    addiu $v0, $v1, 1
    sw $v0, 4($s1)
    lbu $s0, 0($v1)
    addiu $v0, $v1, 2
    sw $v0, 4($s1)
    lbu $s4, 1($v1)
    jal audio_sequence_read_delta
    addu $a1, $s3, $zero
    addu $a0, $s2, $zero
    addu $a1, $s3, $zero
    sw $v0, 0x88($s1)
    lui $v0, %hi(audio_sequence_voice_globals + 0x8)
    lw $v0, %lo(audio_sequence_voice_globals + 0x8)($v0)
    j .Lscheduler_8016d30c
    addu $a2, $s0, $zero
.Lscheduler_8016d1fc:
    lw $v1, 4($s1)
    sb $v0, 0x11($s1)
    addiu $v0, $v1, 1
    sw $v0, 4($s1)
    lbu $a2, 0($v1)
    lui $v0, %hi(audio_sequence_voice_globals + 0x18)
    lw $v0, %lo(audio_sequence_voice_globals + 0x18)($v0)
    j .Lscheduler_8016d394
    addu $a0, $s2, $zero
.Lscheduler_8016d220:
    lw $v1, 4($s1)
    addiu $v0, $zero, 0xc0
    sb $v0, 0x11($s1)
    addiu $v0, $v1, 1
    sw $v0, 4($s1)
    lbu $a2, 0($v1)
    lui $v0, %hi(audio_sequence_voice_globals + 0xc)
    lw $v0, %lo(audio_sequence_voice_globals + 0xc)($v0)
    j .Lscheduler_8016d394
    addu $a0, $s2, $zero
.Lscheduler_8016d248:
    addu $a0, $s2, $zero
    lw $v0, 4($s1)
    addiu $v1, $zero, 0xe0
    sb $v1, 0x11($s1)
    addiu $v0, $v0, 1
    j .Lscheduler_8016d350
    sw $v0, 4($s1)
.Lscheduler_8016d264:
    lw $v1, 4($s1)
    sb $v0, 0x11($s1)
    sb $a0, 0x12($s1)
    addiu $v0, $v1, 1
    sw $v0, 4($s1)
    lbu $a2, 0($v1)
    addiu $v0, $zero, 0x2f
    beq $a2, $v0, .Lscheduler_8016d378
    addu $a0, $s2, $zero
    j .Lscheduler_8016d38c
    nop
.Lscheduler_8016d290:
    lbu $v1, 0x11($s1)
    addiu $v0, $zero, 0xc0
    beq $v1, $v0, .Lscheduler_8016d334
    slti $v0, $v1, 0xc1
    beqz $v0, .Lscheduler_8016d2c0
    addiu $v0, $zero, 0x90
    beq $v1, $v0, .Lscheduler_8016d2dc
    addiu $v0, $zero, 0xb0
    beq $v1, $v0, .Lscheduler_8016d320
    addu $a0, $s2, $zero
    j .Lscheduler_8016d3a0
    nop
.Lscheduler_8016d2c0:
    addiu $v0, $zero, 0xe0
    beq $v1, $v0, .Lscheduler_8016d34c
    addiu $v0, $zero, 0xff
    beq $v1, $v0, .Lscheduler_8016d36c
    andi $a2, $s0, 0xff
    j .Lscheduler_8016d3a0
    nop
.Lscheduler_8016d2dc:
    addu $a0, $s2, $zero
    addiu $v0, $a3, 2
    sw $v0, 4($s1)
    lbu $s4, 1($a3)
    jal audio_sequence_read_delta
    addu $a1, $s3, $zero
    addu $a0, $s2, $zero
    addu $a1, $s3, $zero
    addu $a2, $s0, $zero
    sw $v0, 0x88($s1)
    lui $v0, %hi(audio_sequence_voice_globals + 0x8)
    lw $v0, %lo(audio_sequence_voice_globals + 0x8)($v0)
.Lscheduler_8016d30c:
    nop
    jalr $v0
    addu $a3, $s4, $zero
    j .Lscheduler_8016d3a0
    nop
.Lscheduler_8016d320:
    addu $a1, $s3, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 0x18)
    lw $v0, %lo(audio_sequence_voice_globals + 0x18)($v0)
    j .Lscheduler_8016d398
    addu $a2, $s0, $zero
.Lscheduler_8016d334:
    addu $a0, $s2, $zero
    addu $a1, $s3, $zero
    lui $v0, %hi(audio_sequence_voice_globals + 0xc)
    lw $v0, %lo(audio_sequence_voice_globals + 0xc)($v0)
    j .Lscheduler_8016d398
    addu $a2, $s0, $zero
.Lscheduler_8016d34c:
    addu $a0, $s2, $zero
.Lscheduler_8016d350:
    lui $v0, %hi(audio_sequence_voice_globals + 0x10)
    lw $v0, %lo(audio_sequence_voice_globals + 0x10)($v0)
    nop
    jalr $v0
    addu $a1, $s3, $zero
    j .Lscheduler_8016d3a0
    nop
.Lscheduler_8016d36c:
    addiu $v0, $zero, 0x2f
    bne $a2, $v0, .Lscheduler_8016d38c
    addu $a0, $s2, $zero
.Lscheduler_8016d378:
    addu $a1, $s3, $zero
    jal audio_sequence_end_event
    addiu $a2, $zero, 0x2f
    j .Lscheduler_8016d3a0
    nop
.Lscheduler_8016d38c:
    lui $v0, %hi(audio_sequence_voice_globals + 0x14)
    lw $v0, %lo(audio_sequence_voice_globals + 0x14)($v0)
.Lscheduler_8016d394:
    addu $a1, $s3, $zero
.Lscheduler_8016d398:
    jalr $v0
    nop
.Lscheduler_8016d3a0:
    lw $ra, 0x24($sp)
    lw $s4, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x28
    jr $ra
    nop
.end audio_sequence_decode_event

.section .text
.balign 4
# exe/slus_004_22@0x8016D3C4..0x8016D4CC
.globl audio_sequence_start_linked_record
.ent audio_sequence_start_linked_record
audio_sequence_start_linked_record:
    sll $a0, $a0, 0x10
    lui $v0, %hi(audio_sequence_handles)
    addiu $v0, $v0, %lo(audio_sequence_handles)
    sra $a0, $a0, 0xe
    addu $a0, $a0, $v0
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
    sll $a2, $a1, 1
    addu $a2, $a2, $a1
    sll $a2, $a2, 2
    subu $a2, $a2, $a1
    sll $a2, $a2, 2
    subu $a2, $a2, $a1
    sll $a2, $a2, 2
    lw $a3, 0($a0)
    addiu $v0, $zero, 1
    addu $a3, $a3, $a2
    sh $v0, 0x46($a3)
    sh $zero, 0x48($a3)
    lw $v1, 0($a0)
    nop
    addu $v1, $a2, $v1
    lw $v0, 0x90($v1)
    addiu $a1, $zero, -0x101
    and $v0, $v0, $a1
    sw $v0, 0x90($v1)
    lw $v1, 0($a0)
    nop
    addu $v1, $a2, $v1
    lw $v0, 0x90($v1)
    addiu $a1, $zero, -9
    and $v0, $v0, $a1
    sw $v0, 0x90($v1)
    lw $v1, 0($a0)
    nop
    addu $v1, $a2, $v1
    lw $v0, 0x90($v1)
    addiu $a1, $zero, -3
    and $v0, $v0, $a1
    sw $v0, 0x90($v1)
    lw $v1, 0($a0)
    nop
    addu $v1, $a2, $v1
    lw $v0, 0x90($v1)
    addiu $a1, $zero, -5
    and $v0, $v0, $a1
    sw $v0, 0x90($v1)
    lw $v1, 0($a0)
    nop
    addu $v1, $a2, $v1
    lw $v0, 0x90($v1)
    addiu $a1, $zero, -0x201
    and $v0, $v0, $a1
    sw $v0, 0x90($v1)
    lw $v1, 8($a3)
    addiu $v0, $zero, 1
    sb $v0, 0x2b($a3)
    sw $v1, 4($a3)
    lw $v0, 0($a0)
    nop
    addu $a2, $a2, $v0
    lw $v0, 0x90($a2)
    nop
    ori $v0, $v0, 1
    jr $ra
    sw $v0, 0x90($a2)
.end audio_sequence_start_linked_record

.section .text
.balign 4
# exe/slus_004_22@0x8016D4CC..0x8016D534
.globl audio_sequence_resume_record
.ent audio_sequence_resume_record
audio_sequence_resume_record:
    sll $a0, $a0, 0x10
    lui $v0, %hi(audio_sequence_handles)
    addiu $v0, $v0, %lo(audio_sequence_handles)
    sra $a0, $a0, 0xe
    addu $a0, $a0, $v0
    sll $a1, $a1, 0x10
    sra $a1, $a1, 0x10
    sll $v0, $a1, 1
    addu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sll $v0, $v0, 2
    subu $v0, $v0, $a1
    sll $v0, $v0, 2
    lw $v1, 0($a0)
    addiu $a1, $zero, 1
    addu $v1, $v1, $v0
    sb $a1, 0x2b($v1)
    lw $v1, 0($a0)
    nop
    addu $v0, $v0, $v1
    lw $v1, 0x90($v0)
    addiu $a0, $zero, -9
    and $v1, $v1, $a0
    jr $ra
    sw $v1, 0x90($v0)
.end audio_sequence_resume_record

.section .text
.balign 4
# exe/slus_004_22@0x8016D534..0x8016D69C
.globl audio_sequence_stop_record
.ent audio_sequence_stop_record
audio_sequence_stop_record:
    addiu $sp, $sp, -0x18
    sll $t0, $a0, 0x10
    lui $v0, %hi(audio_sequence_handles)
    addiu $v0, $v0, %lo(audio_sequence_handles)
    sra $t0, $t0, 0xe
    addu $t0, $t0, $v0
    sll $v0, $a1, 0x10
    sra $v0, $v0, 0x10
    sll $v1, $v0, 1
    addu $v1, $v1, $v0
    sll $v1, $v1, 2
    subu $v1, $v1, $v0
    sll $v1, $v1, 2
    subu $v1, $v1, $v0
    sw $ra, 0x14($sp)
    sw $s0, 0x10($sp)
    lw $v0, 0($t0)
    sll $v1, $v1, 2
    addu $s0, $v0, $v1
    lw $v0, 0x90($s0)
    addiu $a2, $zero, -2
    and $v0, $v0, $a2
    sw $v0, 0x90($s0)
    lw $a2, 0($t0)
    addiu $a3, $zero, -3
    addu $a2, $v1, $a2
    lw $v0, 0x90($a2)
    sll $a1, $a1, 8
    and $v0, $v0, $a3
    sw $v0, 0x90($a2)
    lw $a2, 0($t0)
    or $a0, $a0, $a1
    addu $a2, $v1, $a2
    lw $v0, 0x90($a2)
    addiu $a3, $zero, -9
    and $v0, $v0, $a3
    sw $v0, 0x90($a2)
    lw $v0, 0($t0)
    sll $a0, $a0, 0x10
    addu $v1, $v1, $v0
    lw $v0, 0x90($v1)
    sra $a0, $a0, 0x10
    ori $v0, $v0, 4
    jal audio_sequence_release_owner_voices
    sw $v0, 0x90($v1)
    addu $a3, $zero, $zero
    addiu $t2, $zero, 0x40
    addiu $t1, $zero, 0x7f
    lw $v0, 0x7c($s0)
    lw $v1, 0x84($s0)
    lhu $a0, 0x72($s0)
    lw $a1, 8($s0)
    lw $a2, 8($s0)
    addu $t0, $s0, $zero
    sb $zero, 0x2b($s0)
    sw $zero, 0x80($s0)
    sb $zero, 0x27($s0)
    sb $zero, 0x13($s0)
    sb $zero, 0x14($s0)
    sb $zero, 0x29($s0)
    sb $zero, 0x15($s0)
    sb $zero, 0x16($s0)
    sb $zero, 0x2a($s0)
    sb $zero, 0x12($s0)
    sh $zero, 0x48($s0)
    sb $zero, 0x27($s0)
    sb $zero, 0x28($s0)
    sb $zero, 0x10($s0)
    sb $zero, 0x11($s0)
    sw $v0, 0x88($s0)
    sw $v1, 0x8c($s0)
    sh $a0, 0x70($s0)
    sw $a1, 4($s0)
    sw $a2, 0xc($s0)
.Lscheduler_8016d65c:
    addu $v0, $s0, $a3
    sb $a3, 0x2c($v0)
    sb $t2, 0x17($v0)
    sh $t1, 0x4e($t0)
    addiu $a3, $a3, 1
    slti $v0, $a3, 0x10
    bnez $v0, .Lscheduler_8016d65c
    addiu $t0, $t0, 2
    addiu $v0, $zero, 0x7f
    sh $v0, 0x78($s0)
    sh $v0, 0x7a($s0)
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_stop_record

.section .text
.balign 4
# exe/slus_004_22@0x8016D69C..0x8016D6C4
.globl audio_sequence_stop
.ent audio_sequence_stop
audio_sequence_stop:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    sll $a0, $a0, 0x10
    sra $a0, $a0, 0x10
    jal audio_sequence_stop_record
    addu $a1, $zero, $zero
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_stop

.section .text
.balign 4
# exe/slus_004_22@0x8016D6C4..0x8016D6F0
.globl audio_sequence_stop_wrapper
.ent audio_sequence_stop_wrapper
audio_sequence_stop_wrapper:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    sll $a0, $a0, 0x10
    sll $a1, $a1, 0x10
    sra $a0, $a0, 0x10
    jal audio_sequence_stop_record
    sra $a1, $a1, 0x10
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sequence_stop_wrapper

.section .text
.balign 4
# exe/slus_004_22@0x8016D6F0..0x8016D7EC
.globl audio_sequence_set_output_gain
.ent audio_sequence_set_output_gain
audio_sequence_set_output_gain:
    addiu $sp, $sp, -0x40
    sw $ra, 0x38($sp)
    addu $a3, $a1, $zero
    andi $v0, $a0, 0xff
    bnez $v0, .Lscheduler_8016d768
    addu $t0, $a2, $zero
    addiu $v0, $zero, 0xc0
    sw $v0, 0x10($sp)
    sll $v0, $a1, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x80
    bnez $v0, .Lscheduler_8016d728
    sll $v0, $a2, 0x10
    addiu $a3, $zero, 0x7f
.Lscheduler_8016d728:
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x80
    bnez $v0, .Lscheduler_8016d73c
    sll $v1, $a3, 0x10
    addiu $t0, $zero, 0x7f
.Lscheduler_8016d73c:
    sra $v1, $v1, 0x10
    sll $v0, $v1, 7
    addu $v0, $v0, $v1
    sll $v0, $v0, 1
    sll $v1, $t0, 0x10
    sra $v1, $v1, 0x10
    sh $v0, 0x20($sp)
    sll $v0, $v1, 7
    addu $v0, $v0, $v1
    sll $v0, $v0, 1
    sh $v0, 0x22($sp)
.Lscheduler_8016d768:
    andi $v1, $a0, 0xff
    addiu $v0, $zero, 1
    bne $v1, $v0, .Lscheduler_8016d7d4
    addiu $v0, $zero, 0xc00
    sw $v0, 0x10($sp)
    sll $v0, $a3, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x80
    bnez $v0, .Lscheduler_8016d794
    sll $v0, $t0, 0x10
    addiu $a3, $zero, 0x7f
.Lscheduler_8016d794:
    sra $v0, $v0, 0x10
    slti $v0, $v0, 0x80
    bnez $v0, .Lscheduler_8016d7a8
    sll $v1, $a3, 0x10
    addiu $t0, $zero, 0x7f
.Lscheduler_8016d7a8:
    sra $v1, $v1, 0x10
    sll $v0, $v1, 7
    addu $v0, $v0, $v1
    sll $v0, $v0, 1
    sll $v1, $t0, 0x10
    sra $v1, $v1, 0x10
    sh $v0, 0x2c($sp)
    sll $v0, $v1, 7
    addu $v0, $v0, $v1
    sll $v0, $v0, 1
    sh $v0, 0x2e($sp)
.Lscheduler_8016d7d4:
    jal audio_spu_set_common_attributes
    addiu $a0, $sp, 0x10
    lw $ra, 0x38($sp)
    addiu $sp, $sp, 0x40
    jr $ra
    nop
.end audio_sequence_set_output_gain

.section .text
.balign 4
# exe/slus_004_22@0x8016DC0C..0x8016DDEC
.globl audio_sequence_ramp_tempo
.ent audio_sequence_ramp_tempo
audio_sequence_ramp_tempo:
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
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_handles)
    addu $at, $at, $a2
    lw $v1, %lo(audio_sequence_handles)($at)
    addu $a2, $a0, $zero
    addu $a3, $v1, $v0
    lw $v0, 0xa0($a3)
    lh $a0, 0x44($a3)
    addiu $v0, $v0, -1
    blez $a0, .Lscheduler_8016dcac
    sw $v0, 0xa0($a3)
    divu $zero, $v0, $a0
    bnez $a0, .Lscheduler_8016dc70
    nop
    break 7
.Lscheduler_8016dc70:
    mfhi $v0
    nop
    bnez $v0, .Lscheduler_8016dde4
    nop
    lw $v1, 0x8c($a3)
    lw $a0, 0xa4($a3)
    nop
    sltu $v0, $a0, $v1
    bnez $v0, .Lscheduler_8016dca4
    addiu $v0, $v1, -1
    sltu $v0, $v1, $a0
    beqz $v0, .Lscheduler_8016dcf4
    addiu $v0, $v1, 1
.Lscheduler_8016dca4:
    j .Lscheduler_8016dcf4
    sw $v0, 0x8c($a3)
.Lscheduler_8016dcac:
    lw $v1, 0x8c($a3)
    lw $t0, 0xa4($a3)
    nop
    sltu $v0, $t0, $v1
    beqz $v0, .Lscheduler_8016dcd0
    addu $v0, $v1, $a0
    sw $v0, 0x8c($a3)
    j .Lscheduler_8016dce8
    sltu $v0, $v0, $t0
.Lscheduler_8016dcd0:
    sltu $v0, $v1, $t0
    beqz $v0, .Lscheduler_8016dcf4
    subu $v0, $v1, $a0
    lw $t0, 0xa4($a3)
    sw $v0, 0x8c($a3)
    sltu $v0, $t0, $v0
.Lscheduler_8016dce8:
    beqz $v0, .Lscheduler_8016dcf4
    nop
    sw $t0, 0x8c($a3)
.Lscheduler_8016dcf4:
    lh $v0, 0x4a($a3)
    lw $v1, 0x8c($a3)
    nop
    mult $v0, $v1
    lui $a0, %hi(audio_sequence_tick_rate)
    lw $a0, %lo(audio_sequence_tick_rate)($a0)
    mflo $v0
    sll $v1, $v0, 2
    addu $v1, $v1, $v0
    sll $v1, $v1, 1
    sll $v0, $a0, 4
    subu $v0, $v0, $a0
    sll $v0, $v0, 2
    divu $zero, $v1, $v0
    bnez $v0, .Lscheduler_8016dd38
    nop
    break 7
.Lscheduler_8016dd38:
    mflo $v1
    nop
    sh $v1, 0x70($a3)
    sll $v1, $v1, 0x10
    bgtz $v1, .Lscheduler_8016dd54
    addiu $v0, $zero, 1
    sh $v0, 0x70($a3)
.Lscheduler_8016dd54:
    lw $v0, 0xa0($a3)
    nop
    beqz $v0, .Lscheduler_8016dd78
    nop
    lw $v1, 0x8c($a3)
    lw $v0, 0xa4($a3)
    nop
    bne $v1, $v0, .Lscheduler_8016dde4
    nop
.Lscheduler_8016dd78:
    sll $a2, $a2, 0x10
    lui $v0, %hi(audio_sequence_handles)
    addiu $v0, $v0, %lo(audio_sequence_handles)
    sra $a2, $a2, 0xe
    addu $a2, $a2, $v0
    sll $v1, $a1, 0x10
    sra $v1, $v1, 0x10
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    sll $v0, $v0, 2
    subu $v0, $v0, $v1
    lw $a1, 0($a2)
    sll $v0, $v0, 2
    addu $a1, $v0, $a1
    lw $v1, 0x90($a1)
    addiu $a0, $zero, -0x41
    and $v1, $v1, $a0
    sw $v1, 0x90($a1)
    lw $v1, 0($a2)
    nop
    addu $v0, $v0, $v1
    lw $v1, 0x90($v0)
    addiu $a0, $zero, -0x81
    and $v1, $v1, $a0
    sw $v1, 0x90($v0)
.Lscheduler_8016dde4:
    jr $ra
    nop
.end audio_sequence_ramp_tempo

.section .data
.balign 4

# exe/slus_004_22@0x8014A084, eight live code pointers.
.globl audio_spu_common_left_modes
audio_spu_common_left_modes:
    .word .Lscheduler_8016bdc0
    .word .Lscheduler_8016bd88
    .word .Lscheduler_8016bd90
    .word .Lscheduler_8016bd98
    .word .Lscheduler_8016bda0
    .word .Lscheduler_8016bda8
    .word .Lscheduler_8016bdb0
    .word .Lscheduler_8016bdb8

# exe/slus_004_22@0x8014A0A4, eight live code pointers.
.globl audio_spu_common_right_modes
audio_spu_common_right_modes:
    .word .Lscheduler_8016be88
    .word .Lscheduler_8016be50
    .word .Lscheduler_8016be58
    .word .Lscheduler_8016be60
    .word .Lscheduler_8016be68
    .word .Lscheduler_8016be70
    .word .Lscheduler_8016be78
    .word .Lscheduler_8016be80
