# SPDX-License-Identifier: MIT
# Owned A0/72 service. Startup loads this image and zeroes its BSS.
# State holds five live EvCB handles followed by two live ExCB node pointers.
# The caller owns event/exception/thread descriptors and exception delivery.
# O32 return preserves SP, RA, S0/S1 and leaves the critical section entered.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_cd_remove
.ent audio_cd_remove
audio_cd_remove:
    addiu $sp, $sp, -32
    sw $ra, 28($sp)
    sw $s0, 24($sp)
    sw $s1, 20($sp)
    addiu $a0, $zero, 1
    syscall
    lui $s0, %hi(audio_cd_removal_state)
    addiu $s0, $s0, %lo(audio_cd_removal_state)
    addiu $s1, $s0, 20
1:
    lw $a0, 0($s0)
    addiu $s0, $s0, 4
    addiu $t1, $zero, 9
    jal 0x800000B0
    nop
    bne $s0, $s1, 1b
    nop
    lw $a1, 0($s1)
    addiu $a0, $zero, 0
    addiu $t1, $zero, 3
    jal 0x800000C0
    nop
    lw $a1, 4($s1)
    addiu $a0, $zero, 0
    addiu $t1, $zero, 3
    jal 0x800000C0
    nop
    lw $ra, 28($sp)
    lw $s0, 24($sp)
    lw $s1, 20($sp)
    addiu $sp, $sp, 32
    jr $ra
    nop
.end audio_cd_remove

.section .data
.balign 4
.globl audio_cd_removal_state
audio_cd_removal_state:
    .word 0, 0, 0, 0, 0
    .word audio_cd_irq_node, audio_cd_dma_node

.section .bss
.balign 4
.globl audio_cd_irq_node
audio_cd_irq_node:
    .space 16
.globl audio_cd_dma_node
audio_cd_dma_node:
    .space 16
