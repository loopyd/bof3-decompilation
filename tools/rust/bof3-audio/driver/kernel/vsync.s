# SPDX-License-Identifier: MIT
# Owned VSync register polling and software-counter wait.
# Provenance: exe/slus_004_22@0x80174700..0x801748E4, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_vsync
.ent audio_vsync
audio_vsync:
    lui $v0, %hi(audio_vsync_registers)
    lw $v0, %lo(audio_vsync_registers)($v0)
    lui $v1, %hi(audio_vsync_registers + 4)
    lw $v1, %lo(audio_vsync_registers + 4)($v1)
    addiu $sp, $sp, -0x20
    sw $ra, 0x18($sp)
    sw $s1, 0x14($sp)
    sw $s0, 0x10($sp)
    lw $s0, 0($v0)
    lw $v0, 0($v1)
    lui $v1, %hi(audio_vsync_registers + 8)
    lw $v1, %lo(audio_vsync_registers + 8)($v1)
    nop
    subu $v0, $v0, $v1
    bgez $a0, .Lvsync_80174750
    andi $s1, $v0, 0xffff
    lui $v0, %hi(audio_callbacks_vblank_count)
    lw $v0, %lo(audio_callbacks_vblank_count)($v0)
    j .Lvsync_80174830
    nop
.Lvsync_80174750:
    addiu $v0, $zero, 1
    beq $a0, $v0, .Lvsync_80174830
    addu $v0, $s1, $zero
    blez $a0, .Lvsync_8017477c
    nop
    lui $v0, %hi(audio_vsync_registers + 12)
    lw $v0, %lo(audio_vsync_registers + 12)($v0)
    nop
    addiu $v0, $v0, -1
    j .Lvsync_80174784
    addu $v0, $v0, $a0
.Lvsync_8017477c:
    lui $v0, %hi(audio_vsync_registers + 12)
    lw $v0, %lo(audio_vsync_registers + 12)($v0)
.Lvsync_80174784:
    blez $a0, .Lvsync_80174790
    addu $a1, $zero, $zero
    addiu $a1, $a0, -1
.Lvsync_80174790:
    jal audio_vsync_wait
    addu $a0, $v0, $zero
    lui $v0, %hi(audio_vsync_registers)
    lw $v0, %lo(audio_vsync_registers)($v0)
    nop
    lw $s0, 0($v0)
    lui $a0, %hi(audio_callbacks_vblank_count)
    lw $a0, %lo(audio_callbacks_vblank_count)($a0)
    addiu $a1, $zero, 1
    jal audio_vsync_wait
    addiu $a0, $a0, 1
    lui $v0, 0x40
    and $v0, $s0, $v0
    beqz $v0, .Lvsync_80174808
    nop
    lui $v1, %hi(audio_vsync_registers)
    lw $v1, %lo(audio_vsync_registers)($v1)
    nop
    lw $v0, 0($v1)
    nop
    xor $v0, $s0, $v0
    bltz $v0, .Lvsync_80174808
    nop
    lui $a0, 0x8000
.Lvsync_801747f0:
    lw $v0, 0($v1)
    nop
    xor $v0, $s0, $v0
    and $v0, $v0, $a0
    beqz $v0, .Lvsync_801747f0
    nop
.Lvsync_80174808:
    lui $v0, %hi(audio_callbacks_vblank_count)
    lw $v0, %lo(audio_callbacks_vblank_count)($v0)
    lui $v1, %hi(audio_vsync_registers + 4)
    lw $v1, %lo(audio_vsync_registers + 4)($v1)
    lui $at, %hi(audio_vsync_registers + 12)
    sw $v0, %lo(audio_vsync_registers + 12)($at)
    lw $v1, 0($v1)
    addu $v0, $s1, $zero
    lui $at, %hi(audio_vsync_registers + 8)
    sw $v1, %lo(audio_vsync_registers + 8)($at)
.Lvsync_80174830:
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_vsync
.globl audio_vsync_wait
.ent audio_vsync_wait
audio_vsync_wait:
    addiu $sp, $sp, -0x20
    sll $a1, $a1, 0xf
    sw $a1, 0x10($sp)
    lui $v0, %hi(audio_callbacks_vblank_count)
    lw $v0, %lo(audio_callbacks_vblank_count)($v0)
    nop
    slt $v0, $v0, $a0
    beqz $v0, .Lvsync_801748d4
    sw $ra, 0x18($sp)
    addiu $v1, $zero, -1
.Lvsync_80174870:
    lw $v0, 0x10($sp)
    nop
    addiu $v0, $v0, -1
    sw $v0, 0x10($sp)
    lw $v0, 0x10($sp)
    nop
    bne $v0, $v1, .Lvsync_801748bc
    nop
    lui $a0, %hi(.Lvsync_timeout)
    addiu $a0, $a0, %lo(.Lvsync_timeout)
    jal audio_cd_service_puts
    nop
    jal audio_callbacks_clear_pad
    addu $a0, $zero, $zero
    addiu $a0, $zero, 3
    jal audio_callbacks_clear_counter
    addu $a1, $zero, $zero
    j .Lvsync_801748d4
    nop
.Lvsync_801748bc:
    lui $v0, %hi(audio_callbacks_vblank_count)
    lw $v0, %lo(audio_callbacks_vblank_count)($v0)
    nop
    slt $v0, $v0, $a0
    bnez $v0, .Lvsync_80174870
    nop
.Lvsync_801748d4:
    lw $ra, 0x18($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_vsync_wait
.section .data
.balign 4
# exe/slus_004_22@0x801845F8..0x80184608.
.globl audio_vsync_registers
audio_vsync_registers:
    .word 0x1f801814, 0x1f801110, 0, 0
.Lvsync_timeout:
    .asciz "VSync: timeout\n"
