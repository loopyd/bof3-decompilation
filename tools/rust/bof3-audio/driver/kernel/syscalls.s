# SPDX-License-Identifier: MIT
# Ordinary priority-zero ExCB verifier. The owned BFC0011C guard validates the
# live PCB/TCB and supported IRQ/syscall cause before returning the TCB in V0.
# Guest instructions update the saved context. B0/7 delivers unknown syscalls;
# B0/17 restores the live current thread and abandons this exception stack.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_syscall_handler
.ent audio_syscall_handler
audio_syscall_handler:
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    lui $t0, 0xbfc0
    ori $t0, $t0, 0x011c
    jalr $t0
    nop
    lw $t0, 0x98($v0)
    nop
    andi $t0, $t0, 0x3c
    beq $t0, $zero, .Lirq
    nop
    lw $t0, 0x88($v0)
    lw $t1, 0x18($v0)
    addiu $t0, $t0, 4
    sw $t0, 0x88($v0)
    beq $t1, $zero, .Lreturn
    addiu $t0, $zero, 1
    beq $t1, $t0, .Lcritical_enter
    addiu $t0, $zero, 2
    beq $t1, $t0, .Lcritical_exit
    addiu $t0, $zero, 3
    beq $t1, $t0, .Lswitch
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0x10
    addiu $a1, $zero, 0x4000
    addiu $t1, $zero, 7
    addiu $t0, $zero, 0xb0
    jalr $t0
    nop
    beq $zero, $zero, .Lreturn
    nop
.Lcritical_enter:
    lw $t1, 0x94($v0)
    nop
    andi $t2, $t1, 0x404
    xori $t2, $t2, 0x404
    sltiu $t2, $t2, 1
    sw $t2, 0x10($v0)
    lw $t1, 0x94($v0)
    lui $t2, 0xffff
    ori $t2, $t2, 0xfbfb
    and $t1, $t1, $t2
    sw $t1, 0x94($v0)
    beq $zero, $zero, .Lreturn
    nop
.Lcritical_exit:
    lw $t1, 0x94($v0)
    nop
    ori $t1, $t1, 0x404
    sw $t1, 0x94($v0)
    beq $zero, $zero, .Lreturn
    nop
.Lswitch:
    lw $t1, 0x1c($v0)
    lw $t2, 0x108($zero)
    nop
    sw $t1, 0($t2)
    addiu $t1, $zero, 1
    sw $t1, 0x10($v0)
.Lreturn:
    addiu $t1, $zero, 0x17
    addiu $t0, $zero, 0xb0
    jr $t0
    nop
.Lirq:
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    addu $v0, $zero, $zero
.end audio_syscall_handler
