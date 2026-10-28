# SPDX-License-Identifier: MIT
# Ordinary ExCB handlers. Startup installs the exported entrypoints and binds
# audio_counter_flags to ChangeClearRCnt. The linked image owns all fifteen
# acknowledgement words. Callbacks may edit them before acknowledgement.
# B0/7 delivers events; B0/17 abandons the active exception and its stack.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_counter_0_verify
.globl audio_counter_0_acknowledge
.globl audio_counter_1_verify
.globl audio_counter_1_acknowledge
.globl audio_counter_2_verify
.globl audio_counter_2_acknowledge
.globl audio_vblank_verify
.globl audio_vblank_acknowledge
audio_counter_0_verify:
    addiu $a2, $zero, 0
    beq $zero, $zero, .Lcounter_verify
    addiu $a3, $zero, 0x10
audio_counter_0_acknowledge:
    addiu $a2, $zero, 0
    beq $zero, $zero, .Lcounter_acknowledge
    addiu $a3, $zero, 0x10
audio_counter_1_verify:
    addiu $a2, $zero, 1
    beq $zero, $zero, .Lcounter_verify
    addiu $a3, $zero, 0x20
audio_counter_1_acknowledge:
    addiu $a2, $zero, 1
    beq $zero, $zero, .Lcounter_acknowledge
    addiu $a3, $zero, 0x20
audio_counter_2_verify:
    addiu $a2, $zero, 2
    beq $zero, $zero, .Lcounter_verify
    addiu $a3, $zero, 0x40
audio_counter_2_acknowledge:
    addiu $a2, $zero, 2
    beq $zero, $zero, .Lcounter_acknowledge
    addiu $a3, $zero, 0x40
audio_vblank_verify:
    addiu $a2, $zero, 3
    beq $zero, $zero, .Lcounter_verify
    addiu $a3, $zero, 1
audio_vblank_acknowledge:
    addiu $a2, $zero, 3
    beq $zero, $zero, .Lcounter_acknowledge
    addiu $a3, $zero, 1
.Lcounter_verify:
    lui $t0, 0x1f80
    lw $t1, 0x1074($t0)
    nop
    and $t1, $t1, $a3
    beq $t1, $zero, .Lcounter_unmatched
    nop
    lw $t1, 0x1070($t0)
    nop
    and $t1, $t1, $a3
    beq $t1, $zero, .Lcounter_unmatched
    nop
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    lui $a0, 0xf200
    or $a0, $a0, $a2
    addiu $a1, $zero, 2
    addiu $t1, $zero, 7
    addiu $t0, $zero, 0xb0
    jalr $t0
    nop
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    addiu $v0, $zero, 1
.Lcounter_unmatched:
    jr $ra
    addu $v0, $zero, $zero
.Lcounter_acknowledge:
    lui $t0, %hi(audio_counter_flags)
    addiu $t0, $t0, %lo(audio_counter_flags)
    sll $t1, $a2, 2
    addu $t0, $t0, $t1
    lw $t1, 0($t0)
    nop
    beq $t1, $zero, .Lcounter_continue
    nop
    nor $t1, $a3, $zero
    lui $t0, 0x1f80
    sw $t1, 0x1070($t0)
    addiu $t1, $zero, 0x17
    addiu $t0, $zero, 0xb0
    jr $t0
    nop
.Lcounter_continue:
    jr $ra
    nop

# Each source reads I_MASK then I_STAT, including masked sources. A callback
# completes before the next source is read. T0-T2 are rebuilt after each call.
.macro generic_source bit, class
    lui $t0, 0x1f80
    lw $t1, 0x1074($t0)
    lw $t2, 0x1070($t0)
    nop
    and $t1, $t1, $t2
    andi $t1, $t1, (1 << \bit)
    beq $t1, $zero, .Lgeneric_next\@
    nop
    lui $a0, 0xf000
    ori $a0, $a0, \class
    addiu $a1, $zero, 0x1000
    addiu $t1, $zero, 7
    addiu $t0, $zero, 0xb0
    jalr $t0
    nop
.Lgeneric_next\@:
.endm
.globl audio_generic_irq
.ent audio_generic_irq
audio_generic_irq:
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    generic_source 2, 3
    generic_source 9, 9
    generic_source 1, 2
    generic_source 10, 10
    generic_source 8, 11
    generic_source 0, 1
    generic_source 4, 5
    generic_source 5, 6
    generic_source 6, 6
    generic_source 7, 8
    generic_source 3, 4
    lui $t0, %hi(audio_generic_flags)
    addiu $t0, $t0, %lo(audio_generic_flags)
    addiu $t1, $zero, 11
    addiu $t2, $zero, 1
    addu $t3, $zero, $zero
.Lgeneric_flag:
    lw $t4, 0($t0)
    nop
    beq $t4, $zero, .Lgeneric_flag_next
    nop
    or $t3, $t3, $t2
.Lgeneric_flag_next:
    addiu $t0, $t0, 4
    sll $t2, $t2, 1
    addiu $t1, $t1, -1
    bne $t1, $zero, .Lgeneric_flag
    nop
    nor $t3, $t3, $zero
    lui $t0, 0x1f80
    sw $t3, 0x1070($t0)
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    addu $v0, $zero, $zero
.end audio_generic_irq

.section .data
.balign 4
.globl audio_counter_flags
.type audio_counter_flags, @object
audio_counter_flags:
    .word 1, 1, 1, 1
.size audio_counter_flags, .-audio_counter_flags
.globl audio_generic_flags
.type audio_generic_flags, @object
audio_generic_flags:
    .word 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.size audio_generic_flags, .-audio_generic_flags
