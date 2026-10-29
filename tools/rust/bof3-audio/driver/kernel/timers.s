# SPDX-License-Identifier: MIT
# Owned counter configuration and reset used by sequence callback registration.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x8017EEEC..0x8017EF88
.globl audio_counter_set
.ent audio_counter_set
audio_counter_set:
    andi $t0, $a0, 0xffff
    slti $v0, $t0, 3
    bnez $v0, .Lcounter_8017ef04
    addiu $a3, $zero, 0x48
    j .Lcounter_8017ef80
    addu $v0, $zero, $zero
.Lcounter_8017ef04:
    lui $v0, %hi(audio_counter_registers_pointer)
    lw $v0, %lo(audio_counter_registers_pointer)($v0)
    sll $v1, $t0, 4
    addu $v1, $v1, $v0
    sltiu $v0, $t0, 2
    sh $zero, 4($v1)
    sh $a1, 8($v1)
    beqz $v0, .Lcounter_8017ef44
    andi $v0, $a2, 0x10
    beqz $v0, .Lcounter_8017ef34
    andi $v0, $a2, 1
    addiu $a3, $zero, 0x49
.Lcounter_8017ef34:
    bnez $v0, .Lcounter_8017ef60
    andi $v0, $a2, 0x1000
    j .Lcounter_8017ef60
    ori $a3, $a3, 0x100
.Lcounter_8017ef44:
    addiu $v0, $zero, 2
    bne $t0, $v0, .Lcounter_8017ef60
    andi $v0, $a2, 0x1000
    andi $v0, $a2, 1
    bnez $v0, .Lcounter_8017ef60
    andi $v0, $a2, 0x1000
    addiu $a3, $zero, 0x248
.Lcounter_8017ef60:
    beqz $v0, .Lcounter_8017ef6c
    addiu $v0, $zero, 1
    ori $a3, $a3, 0x10
.Lcounter_8017ef6c:
    lui $a0, %hi(audio_counter_registers_pointer)
    lw $a0, %lo(audio_counter_registers_pointer)($a0)
    sll $v1, $t0, 4
    addu $v1, $v1, $a0
    sh $a3, 4($v1)
.Lcounter_8017ef80:
    jr $ra
    nop
.end audio_counter_set

# exe/slus_004_22@0x8017F02C..0x8017F064
.globl audio_counter_reset
.ent audio_counter_reset
audio_counter_reset:
    andi $v1, $a0, 0xffff
    slti $v0, $v1, 3
    beqz $v0, .Lcounter_8017f058
    addiu $v0, $zero, 1
    lui $a0, %hi(audio_counter_registers_pointer)
    lw $a0, %lo(audio_counter_registers_pointer)($a0)
    sll $v1, $v1, 4
    addu $v1, $v1, $a0
    sh $zero, 0($v1)
    j .Lcounter_8017f05c
    nop
.Lcounter_8017f058:
    addu $v0, $zero, $zero
.Lcounter_8017f05c:
    jr $ra
    nop
.end audio_counter_reset

.section .data
.balign 4
# exe/slus_004_22@0x8018B294
.globl audio_counter_registers_pointer
audio_counter_registers_pointer:
    .word 0x1f801100
