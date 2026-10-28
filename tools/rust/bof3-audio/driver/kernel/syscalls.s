# SPDX-License-Identifier: MIT
# Complete priority-zero verifier with direct calls to owned guest services.
# Entry preflights the live PCB/TCB and the supported IRQ/syscall cause. The
# terminal break retains an explicit failure if nonlocal restoration returns.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_syscall_handler
.ent audio_syscall_handler
audio_syscall_handler:
    lui $t6, 0xa000
    lw $t6, 0x108($t6)
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    lw $v0, 0($t6)
    addu $a0, $t6, $zero
    lw $v1, 0x98($v0)
    beq $zero, $zero, .Lcause_select
    andi $v1, $v1, 0x3c
.Lirq:
    beq $zero, $zero, .Lepilogue
    addu $v0, $zero, $zero
.Lsyscall:
    lw $t7, 0x88($v0)
    lw $v1, 0x18($v0)
    addiu $t8, $t7, 4
    sltiu $at, $v1, 4
    beq $at, $zero, .Lunknown
    sw $t8, 0x88($v0)
    sll $t9, $v1, 2
    lui $at, %hi(audio_syscall_dispatch_table)
    addu $at, $at, $t9
    lw $t9, %lo(audio_syscall_dispatch_table)($at)
    nop
    jr $t9
    nop
.Lcritical_enter:
    lw $t0, 0x94($v0)
    addiu $at, $zero, 0x404
    andi $t1, $t0, 0x404
    bne $t1, $at, .Lcritical_false
    nop
    addiu $t2, $zero, 1
    beq $zero, $zero, .Lcritical_clear
    sw $t2, 0x10($v0)
.Lcritical_false:
    sw $zero, 0x10($v0)
.Lcritical_clear:
    lw $t3, 0x94($v0)
    addiu $at, $zero, -0x405
    and $t4, $t3, $at
    beq $zero, $zero, .Lrestore
    sw $t4, 0x94($v0)
.Lcritical_exit:
    lw $t5, 0x94($v0)
    nop
    ori $t6, $t5, 0x404
    beq $zero, $zero, .Lrestore
    sw $t6, 0x94($v0)
.Lthread_switch:
    lw $t7, 0x1c($v0)
    addiu $t8, $zero, 1
    sw $t7, 0($a0)
    beq $zero, $zero, .Lrestore
    sw $t8, 0x10($v0)
.Lunknown:
    lui $a0, 0xf000
    ori $a0, $a0, 0x10
    jal audio_event_deliver
    addiu $a1, $zero, 0x4000
    beq $zero, $zero, .Lrestore
    nop
.Lrestore:
    jal audio_exception_return
    nop
    beq $zero, $zero, .Lfailure_spec
    lui $a0, 0xf000
.Lcause_select:
    beq $v1, $zero, .Lirq
    addiu $at, $zero, 0x20
    beq $v1, $at, .Lsyscall
    nop
    lui $a0, 0xf000
.Lfailure_spec:
    ori $a0, $a0, 0x10
    jal audio_event_deliver
    addiu $a1, $zero, 0x1000
    jal audio_exception_failure
    nop
.Lepilogue:
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop
.end audio_syscall_handler
.section .data.audio_syscall_dispatch,"aw"
.balign 4
.globl audio_syscall_dispatch_table
audio_syscall_dispatch_table:
    .word .Lrestore, .Lcritical_enter, .Lcritical_exit, .Lthread_switch
.section .text.audio_exception_failure,"ax"
.balign 4
.globl audio_exception_failure
audio_exception_failure:
    break 1
    nop
