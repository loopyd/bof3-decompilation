# SPDX-License-Identifier: MIT
# Forward byte copy from the read-only host media input to guest RAM.
# A0 destination, A1 source, A2 byte count. V0 returns the destination.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_staging_copy
.ent audio_staging_copy
audio_staging_copy:
    addu $v0, $a0, $zero
    beq $a2, $zero, .Lcopy_return
    addu $t0, $a0, $zero
.Lcopy_byte:
    lbu $t1, 0($a1)
    addiu $a1, $a1, 1
    sb $t1, 0($t0)
    addiu $a2, $a2, -1
    bne $a2, $zero, .Lcopy_byte
    addiu $t0, $t0, 1
.Lcopy_return:
    jr $ra
    nop
.end audio_staging_copy

.section .bss
.balign 64
.globl audio_staging_transfer
audio_staging_transfer:
    .space 2048
.globl audio_staging_transfer_end
audio_staging_transfer_end:
