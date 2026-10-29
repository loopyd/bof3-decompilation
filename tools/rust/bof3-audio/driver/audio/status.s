# SPDX-License-Identifier: MIT
# Owned SPU key status and explicit effects status snapshot.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Register base, key bits and status snapshot remain live shared state.
.set noreorder
.set noat

.section .text
.balign 4
# exe/slus_004_22@0x8015D044..0x8015D0B8
.globl audio_effects_refresh_status
.ent audio_effects_refresh_status
audio_effects_refresh_status:
    addiu $sp, $sp, -0x28
    sw $s1, 0x14($sp)
    addu $s1, $zero, $zero
    sw $s3, 0x1c($sp)
    addiu $s3, $zero, 1
    sw $s2, 0x18($sp)
    lui $s2, %hi(audio_sequence_voice_globals + 0x5f0)
    addiu $s2, $s2, %lo(audio_sequence_voice_globals + 0x5f0)
    sw $ra, 0x20($sp)
    sw $s0, 0x10($sp)
    andi $s0, $s1, 0xff
.Leffects_8015d070:
    jal audio_spu_key_status
    sllv $a0, $s3, $s0
    sll $s0, $s0, 2
    addu $s0, $s0, $s2
    sw $v0, 0($s0)
    addiu $s1, $s1, 1
    andi $v0, $s1, 0xff
    sltiu $v0, $v0, 0x18
    bnez $v0, .Leffects_8015d070
    andi $s0, $s1, 0xff
    lw $ra, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x28
    jr $ra
    nop
.end audio_effects_refresh_status

.section .text
.balign 4
# exe/slus_004_22@0x801682E0..0x80168374
.globl audio_spu_key_status
.ent audio_spu_key_status
audio_spu_key_status:
    addiu $a1, $zero, -1
    addu $v1, $zero, $zero
    addiu $a2, $zero, 1
    sllv $v0, $a2, $v1
.Leffects_801682f0:
    and $v0, $a0, $v0
    bnez $v0, .Leffects_80168320
    nop
    addiu $v1, $v1, 1
    slti $v0, $v1, 0x18
    bnez $v0, .Leffects_801682f0
    sllv $v0, $a2, $v1
.Leffects_8016830c:
    addiu $v0, $zero, -1
    bne $a1, $v0, .Leffects_80168328
    sll $a0, $a1, 4
    j .Leffects_8016836c
    nop
.Leffects_80168320:
    j .Leffects_8016830c
    addu $a1, $v1, $zero
.Leffects_80168328:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    lui $v1, %hi(audio_spu_state_3b18)
    lw $v1, %lo(audio_spu_state_3b18)($v1)
    addu $a0, $a0, $v0
    addiu $v0, $zero, 1
    sllv $v0, $v0, $a1
    and $v1, $v1, $v0
    lhu $a0, 0xc($a0)
    beqz $v1, .Leffects_80168364
    nop
    beqz $a0, .Leffects_8016836c
    addiu $v0, $zero, 3
    j .Leffects_8016836c
    addiu $v0, $zero, 1
.Leffects_80168364:
    sltu $v0, $zero, $a0
    sll $v0, $v0, 1
.Leffects_8016836c:
    jr $ra
    nop
.end audio_spu_key_status

.section .data
.balign 4
