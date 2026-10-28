# SPDX-License-Identifier: MIT
# Owned DeliverEvent service. A0 is the event class; A1 is the specification.
# The caller validates the EvCB descriptor at 0x120/0x124 and supplies an O32
# stack. Callbacks preserve the O32 callee-saved registers and return normally,
# or end the active exception through the host kernel's ReturnFromException.
# The table range is captured once. Later records are read after each callback.
# BFC00108 is the owned host callback guard, with the selected target in T9.
# It validates each callback's stack and callee-saved registers before resuming.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_event_deliver
.ent audio_event_deliver
audio_event_deliver:
    addiu $sp, $sp, -40
    sw $ra, 36($sp)
    sw $s0, 32($sp)
    sw $s1, 28($sp)
    sw $s2, 24($sp)
    sw $s3, 20($sp)
    addu $s0, $a0, $zero
    addu $s1, $a1, $zero
    lw $s2, 0x120($zero)
    lw $t0, 0x124($zero)
    nop
    addu $s3, $s2, $t0
.Lnext:
    beq $s2, $s3, .Ldone
    nop
    lw $t0, 4($s2)
    addiu $t1, $zero, 0x2000
    bne $t0, $t1, .Ladvance
    nop
    lw $t0, 0($s2)
    nop
    bne $t0, $s0, .Ladvance
    nop
    lw $t0, 8($s2)
    nop
    bne $t0, $s1, .Ladvance
    nop
    lw $t0, 12($s2)
    nop
    beq $t0, $t1, .Lpoll
    addiu $t1, $zero, 0x1000
    bne $t0, $t1, .Linvalid
    nop
    lw $t9, 16($s2)
    nop
    beq $t9, $zero, .Ladvance
    nop
    lui $t0, 0xbfc0
    ori $t0, $t0, 0x0108
    jalr $t0
    nop
    beq $zero, $zero, .Ladvance
    nop
.Lpoll:
    addiu $t0, $zero, 0x4000
    sw $t0, 4($s2)
.Ladvance:
    beq $zero, $zero, .Lnext
    addiu $s2, $s2, 28
.Linvalid:
    break 1
.Ldone:
    lw $ra, 36($sp)
    lw $s0, 32($sp)
    lw $s1, 28($sp)
    lw $s2, 24($sp)
    lw $s3, 20($sp)
    addiu $sp, $sp, 40
    jr $ra
    nop
.end audio_event_deliver
