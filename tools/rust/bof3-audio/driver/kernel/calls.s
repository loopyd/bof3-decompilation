# SPDX-License-Identifier: MIT
# Owned host-command dispatcher. The host supplies one entry and eight o32
# argument words while stopped at audio_call_return. Guest instructions own
# register setup, the outgoing frame, JALR, frame restoration and continuation.
.set noreorder
.set noat
.section .text.audio_calls,"ax"
.balign 4
.globl audio_calls_start
.ent audio_calls_start
audio_calls_start:
    jal audio_driver_start
    nop
    b audio_call_return
    nop
.end audio_calls_start

.globl audio_call_enter
.ent audio_call_enter
audio_call_enter:
    lui $t0, %hi(audio_call_arguments)
    addiu $t0, $t0, %lo(audio_call_arguments)
    addiu $sp, $sp, -32
    lw $t1, 20($t0)
    lw $t2, 24($t0)
    sw $t1, 16($sp)
    lw $t1, 28($t0)
    sw $t2, 20($sp)
    lw $t2, 32($t0)
    sw $t1, 24($sp)
    lw $t9, 0($t0)
    sw $t2, 28($sp)
    lw $a0, 4($t0)
    lw $a1, 8($t0)
    lw $a2, 12($t0)
    lw $a3, 16($t0)
    nop
    jalr $t9
    nop
    addiu $sp, $sp, 32
.globl audio_call_return
audio_call_return:
    b audio_call_enter
    nop
.globl audio_calls_end
audio_calls_end:
.end audio_call_enter

.section .data
.balign 4
.globl audio_call_arguments
audio_call_arguments:
    .space 36
