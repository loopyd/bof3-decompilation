# SPDX-License-Identifier: MIT
# Required cold-start service bodies from US SCPH-5501, SHA-256
# 11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef.
# setjmp is ROM 0x2240..0x227C. Other provenance ranges below name the
# BIOS-installed RAM copy. Symbolic references relocate owned state only.
.set noreorder
.set noat
.section .text
.balign 4

.globl audio_kernel_setjmp
.ent audio_kernel_setjmp
audio_kernel_setjmp:
    sw $ra, 0($a0)
    sw $gp, 44($a0)
    sw $sp, 4($a0)
    sw $fp, 8($a0)
    sw $s0, 12($a0)
    sw $s1, 16($a0)
    sw $s2, 20($a0)
    sw $s3, 24($a0)
    sw $s4, 28($a0)
    sw $s5, 32($a0)
    sw $s6, 36($a0)
    sw $s7, 40($a0)
    addu $v0, $zero, $zero
    jr $ra
    nop
.end audio_kernel_setjmp

# HookEntryInt, RAM 0xF20..0xF2C.
.globl audio_kernel_hook_entry
.ent audio_kernel_hook_entry
audio_kernel_hook_entry:
    lui $at, %hi(audio_exception_hook_pointer)
    jr $ra
    sw $a0, %lo(audio_exception_hook_pointer)($at)
.end audio_kernel_hook_entry

# ChangeClearPAD, RAM 0x43D0..0x43E8.
.globl audio_kernel_clear_pad
.ent audio_kernel_clear_pad
audio_kernel_clear_pad:
    lui $v1, %hi(audio_pad_auto_ack)
    lw $v1, %lo(audio_pad_auto_ack)($v1)
    lui $at, %hi(audio_pad_auto_ack)
    sw $a0, %lo(audio_pad_auto_ack)($at)
    jr $ra
    addu $v0, $v1, $zero
.end audio_kernel_clear_pad

# ChangeClearRCnt, RAM 0x15D8..0x15F8.
.globl audio_kernel_clear_counter
.ent audio_kernel_clear_counter
audio_kernel_clear_counter:
    lui $t7, %hi(audio_counter_flags)
    addiu $t7, $t7, %lo(audio_counter_flags)
    sll $t6, $a0, 2
    addu $a2, $t6, $t7
    lw $v1, 0($a2)
    sw $a1, 0($a2)
    jr $ra
    addu $v0, $v1, $zero
.end audio_kernel_clear_counter

# SysDeqIntRP, RAM 0x1444..0x1508. Preserve all original branches,
# exact pointer comparisons and the detached node's old next link.
.globl audio_kernel_dequeue
.ent audio_kernel_dequeue
audio_kernel_dequeue:
    lui $t6, 0xa000
    lw $t6, 0x100($t6)
    sll $t7, $a0, 3
    addu $v0, $t6, $t7
    lw $a2, 0($v0)
    addiu $sp, $sp, -8
    bne $a2, $zero, .Ldequeue_head
    addu $v1, $v0, $zero
    beq $zero, $zero, .Ldequeue_return
    addu $v0, $zero, $zero
.Ldequeue_head:
    bne $a2, $a1, .Ldequeue_nonhead
    nop
    lw $t8, 0($a2)
    addu $v0, $a2, $zero
    beq $zero, $zero, .Ldequeue_return
    sw $t8, 0($v1)
.Ldequeue_nonhead:
    lw $v0, 0($a2)
    nop
    bne $v0, $zero, .Ldequeue_compare
    nop
    beq $zero, $zero, .Ldequeue_return
    addu $v0, $zero, $zero
.Ldequeue_compare:
    bne $a2, $a1, .Ldequeue_next
    nop
    lw $v1, 0($sp)
    beq $zero, $zero, .Ldequeue_unlink
    nop
.Ldequeue_next:
    bne $v0, $zero, .Ldequeue_advance
    addu $v1, $a2, $zero
    lw $v1, 0($sp)
    beq $zero, $zero, .Ldequeue_unlink
    nop
.Ldequeue_previous:
    addu $v1, $a2, $zero
.Ldequeue_advance:
    addu $a2, $v0, $zero
    beq $a2, $a1, .Ldequeue_unlink
    nop
    lw $v0, 0($a2)
    nop
    bne $v0, $zero, .Ldequeue_previous
    nop
.Ldequeue_unlink:
    bne $a2, $a1, .Ldequeue_return
    addu $v0, $zero, $zero
    lw $t9, 0($a2)
    addu $v0, $a2, $zero
    beq $zero, $zero, .Ldequeue_return
    sw $t9, 0($v1)
    addu $v0, $zero, $zero
.Ldequeue_return:
    jr $ra
    addiu $sp, $sp, 8
.end audio_kernel_dequeue

.section .bss
.balign 4
.globl audio_pad_auto_ack
audio_pad_auto_ack:
    .space 4
