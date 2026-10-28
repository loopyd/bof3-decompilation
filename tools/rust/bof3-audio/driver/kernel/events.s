# SPDX-License-Identifier: MIT
# DeliverEvent with the original guest frame, live record reads and JALR ABI.
# Startup binds the scanner, mode, callback and final return boundaries. The
# CPU loop validates ownership without changing guest registers or delay slots.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_event_deliver, audio_event_loop, audio_event_mode
.globl audio_event_callback, audio_event_callback_return
.globl audio_event_return, audio_event_return_delay
.ent audio_event_deliver
audio_event_deliver:
    lui $t6, 0xa000
    lw $t6, 0x124($t6)
    addiu $at, $zero, 28
    divu $zero, $t6, $at
    addiu $sp, $sp, -64
    lui $v1, 0xa000
    lw $v1, 0x120($v1)
    sw $s6, 32($sp)
    sw $s4, 24($sp)
    sw $s5, 28($sp)
    sw $s0, 20($sp)
    addu $s5, $a0, $zero
    addu $s6, $a1, $zero
    sw $ra, 36($sp)
    addu $s0, $v1, $zero
    mflo $t7
    sll $t8, $t7, 3
    subu $t8, $t8, $t7
    sll $t8, $t8, 2
    addu $v0, $v1, $t8
    sltu $at, $v1, $v0
    beq $at, $zero, .Lepilogue
    addu $s4, $v0, $zero
    sw $s1, 48($sp)
    sw $s2, 44($sp)
    sw $s3, 40($sp)
    addiu $s3, $zero, 0x1000
    addiu $s2, $zero, 0x4000
    addiu $s1, $zero, 0x2000
audio_event_loop:
    lw $t9, 4($s0)
    nop
    bne $s1, $t9, audio_event_callback_return
    nop
    lw $t0, 0($s0)
    nop
    bne $s5, $t0, audio_event_callback_return
    nop
    lw $t1, 8($s0)
    nop
    bne $s6, $t1, audio_event_callback_return
    nop
    lw $v0, 12($s0)
    nop
audio_event_mode:
    bne $s1, $v0, .Lcallback_mode
    nop
    beq $zero, $zero, audio_event_callback_return
    sw $s2, 4($s0)
.Lcallback_mode:
    bne $s3, $v0, audio_event_callback_return
    nop
    lw $v0, 16($s0)
    nop
    beq $v0, $zero, audio_event_callback_return
    nop
audio_event_callback:
    jalr $v0
    nop
audio_event_callback_return:
    addiu $s0, $s0, 28
    sltu $at, $s0, $s4
    bne $at, $zero, audio_event_loop
    nop
    lw $s1, 48($sp)
    lw $s2, 44($sp)
    lw $s3, 40($sp)
    nop
.Lepilogue:
    lw $ra, 36($sp)
    lw $s0, 20($sp)
    lw $s4, 24($sp)
    lw $s5, 28($sp)
    lw $s6, 32($sp)
audio_event_return:
    jr $ra
audio_event_return_delay:
    addiu $sp, $sp, 64
.end audio_event_deliver
