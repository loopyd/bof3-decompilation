# SPDX-License-Identifier: MIT
# Complete guest ExCB IRQ routines. Startup loads their linked instructions,
# live MMIO-base pointer and acknowledgement words, and binds the event/return
# services. Callbacks complete before later source reads or acknowledgement.
.set noreorder
.set noat
.section .text.audio_counters,"ax"
.balign 4
.globl audio_counter_0_verify
.ent audio_counter_0_verify
audio_counter_0_verify:
    lui $v0, %hi(audio_irq_registers)
    lw $v0, %lo(audio_irq_registers)($v0)
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    lw $t6, 4($v0)
    nop
    andi $t7, $t6, 0x10
    beq $t7, $zero, .L17cc
    nop
    lw $t8, 0($v0)
    nop
    andi $t9, $t8, 0x10
    bne $t9, $zero, .L17d8
    lui $a0, 0xf200
.L17cc:
    beq $zero, $zero, .L17e4
    addu $v0, $zero, $zero
    lui $a0, 0xf200
.L17d8:
    jal audio_event_deliver
    addiu $a1, $zero, 2
    addiu $v0, $zero, 1
.L17e4:
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop
.end audio_counter_0_verify

.globl audio_counter_1_verify
.ent audio_counter_1_verify
audio_counter_1_verify:
    lui $v0, %hi(audio_irq_registers)
    lw $v0, %lo(audio_irq_registers)($v0)
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    lw $t6, 4($v0)
    nop
    andi $t7, $t6, 0x20
    beq $t7, $zero, .L182c
    nop
    lw $t8, 0($v0)
    nop
    andi $t9, $t8, 0x20
    bne $t9, $zero, .L1838
    lui $a0, 0xf200
.L182c:
    beq $zero, $zero, .L1848
    addu $v0, $zero, $zero
    lui $a0, 0xf200
.L1838:
    ori $a0, $a0, 0x1
    jal audio_event_deliver
    addiu $a1, $zero, 2
    addiu $v0, $zero, 1
.L1848:
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop
.end audio_counter_1_verify

.globl audio_counter_2_verify
.ent audio_counter_2_verify
audio_counter_2_verify:
    lui $v0, %hi(audio_irq_registers)
    lw $v0, %lo(audio_irq_registers)($v0)
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    lw $t6, 4($v0)
    nop
    andi $t7, $t6, 0x40
    beq $t7, $zero, .L1890
    nop
    lw $t8, 0($v0)
    nop
    andi $t9, $t8, 0x40
    bne $t9, $zero, .L189c
    lui $a0, 0xf200
.L1890:
    beq $zero, $zero, .L18ac
    addu $v0, $zero, $zero
    lui $a0, 0xf200
.L189c:
    ori $a0, $a0, 0x2
    jal audio_event_deliver
    addiu $a1, $zero, 2
    addiu $v0, $zero, 1
.L18ac:
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop
.end audio_counter_2_verify

.globl audio_vblank_verify
.ent audio_vblank_verify
audio_vblank_verify:
    lui $v0, %hi(audio_irq_registers)
    lw $v0, %lo(audio_irq_registers)($v0)
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    lw $t6, 4($v0)
    nop
    andi $t7, $t6, 0x1
    beq $t7, $zero, .L18f4
    nop
    lw $t8, 0($v0)
    nop
    andi $t9, $t8, 0x1
    bne $t9, $zero, .L1900
    lui $a0, 0xf200
.L18f4:
    beq $zero, $zero, .L1910
    addu $v0, $zero, $zero
    lui $a0, 0xf200
.L1900:
    ori $a0, $a0, 0x3
    jal audio_event_deliver
    addiu $a1, $zero, 2
    addiu $v0, $zero, 1
.L1910:
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop
.end audio_vblank_verify

.globl audio_counter_0_acknowledge
.ent audio_counter_0_acknowledge
audio_counter_0_acknowledge:
    lui $t6, %hi(audio_counter_flags + 0)
    lw $t6, %lo(audio_counter_flags + 0)($t6)
    addiu $sp, $sp, -24
    beq $t6, $zero, .L1948
    sw $ra, 20($sp)
    lui $t8, %hi(audio_irq_registers)
    lw $t8, %lo(audio_irq_registers)($t8)
    addiu $t7, $zero, -17
    jal audio_exception_return
    sw $t7, 0($t8)
.L1948:
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop
.end audio_counter_0_acknowledge

.globl audio_counter_1_acknowledge
.ent audio_counter_1_acknowledge
audio_counter_1_acknowledge:
    lui $t6, %hi(audio_counter_flags + 4)
    lw $t6, %lo(audio_counter_flags + 4)($t6)
    addiu $sp, $sp, -24
    beq $t6, $zero, .L1980
    sw $ra, 20($sp)
    lui $t8, %hi(audio_irq_registers)
    lw $t8, %lo(audio_irq_registers)($t8)
    addiu $t7, $zero, -33
    jal audio_exception_return
    sw $t7, 0($t8)
.L1980:
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop
.end audio_counter_1_acknowledge

.globl audio_counter_2_acknowledge
.ent audio_counter_2_acknowledge
audio_counter_2_acknowledge:
    lui $t6, %hi(audio_counter_flags + 8)
    lw $t6, %lo(audio_counter_flags + 8)($t6)
    addiu $sp, $sp, -24
    beq $t6, $zero, .L19b8
    sw $ra, 20($sp)
    lui $t8, %hi(audio_irq_registers)
    lw $t8, %lo(audio_irq_registers)($t8)
    addiu $t7, $zero, -65
    jal audio_exception_return
    sw $t7, 0($t8)
.L19b8:
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop
.end audio_counter_2_acknowledge

.globl audio_vblank_acknowledge
.ent audio_vblank_acknowledge
audio_vblank_acknowledge:
    lui $t6, %hi(audio_counter_flags + 12)
    lw $t6, %lo(audio_counter_flags + 12)($t6)
    addiu $sp, $sp, -24
    beq $t6, $zero, .L19f0
    sw $ra, 20($sp)
    lui $t8, %hi(audio_irq_registers)
    lw $t8, %lo(audio_irq_registers)($t8)
    addiu $t7, $zero, -2
    jal audio_exception_return
    sw $t7, 0($t8)
.L19f0:
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop
.end audio_vblank_acknowledge

.section .text.audio_generic,"ax"
.balign 4
.globl audio_generic_irq
.ent audio_generic_irq
audio_generic_irq:
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    lw $t6, 4($a2)
    lw $t7, 0($a2)
    nop
    and $t8, $t6, $t7
    andi $t9, $t8, 0x4
    beq $t9, $zero, .L24a0
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0x3
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L24a0:
    lw $t0, 4($a2)
    lw $t1, 0($a2)
    nop
    and $t2, $t0, $t1
    andi $t3, $t2, 0x200
    beq $t3, $zero, .L24d8
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0x9
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L24d8:
    lw $t4, 4($a2)
    lw $t5, 0($a2)
    nop
    and $t6, $t4, $t5
    andi $t7, $t6, 0x2
    beq $t7, $zero, .L2510
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0x2
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L2510:
    lw $t8, 4($a2)
    lw $t9, 0($a2)
    nop
    and $t0, $t8, $t9
    andi $t1, $t0, 0x400
    beq $t1, $zero, .L2548
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0xa
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L2548:
    lw $t2, 4($a2)
    lw $t3, 0($a2)
    nop
    and $t4, $t2, $t3
    andi $t5, $t4, 0x100
    beq $t5, $zero, .L2580
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0xb
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L2580:
    lw $t6, 4($a2)
    lw $t7, 0($a2)
    nop
    and $t8, $t6, $t7
    andi $t9, $t8, 0x1
    beq $t9, $zero, .L25b8
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0x1
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L25b8:
    lw $t0, 4($a2)
    lw $t1, 0($a2)
    nop
    and $t2, $t0, $t1
    andi $t3, $t2, 0x10
    beq $t3, $zero, .L25f0
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0x5
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L25f0:
    lw $t4, 4($a2)
    lw $t5, 0($a2)
    nop
    and $t6, $t4, $t5
    andi $t7, $t6, 0x20
    beq $t7, $zero, .L2628
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0x6
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L2628:
    lw $t8, 4($a2)
    lw $t9, 0($a2)
    nop
    and $t0, $t8, $t9
    andi $t1, $t0, 0x40
    beq $t1, $zero, .L2660
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0x6
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L2660:
    lw $t2, 4($a2)
    lw $t3, 0($a2)
    nop
    and $t4, $t2, $t3
    andi $t5, $t4, 0x80
    beq $t5, $zero, .L2698
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0x8
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L2698:
    lw $t6, 4($a2)
    lw $t7, 0($a2)
    nop
    and $t8, $t6, $t7
    andi $t9, $t8, 0x8
    beq $t9, $zero, .L26d0
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 0x4
    jal audio_event_deliver
    addiu $a1, $zero, 4096
    lui $a2, %hi(audio_irq_registers)
    lw $a2, %lo(audio_irq_registers)($a2)
    nop
.L26d0:
    lui $v1, %hi(audio_generic_flags)
    addu $a0, $zero, $zero
    addiu $v1, $v1, %lo(audio_generic_flags)
    addu $v0, $zero, $zero
    addiu $a1, $zero, 11
.L26e4:
    lw $t0, 0($v1)
    nop
    beq $t0, $zero, .L2700
    nop
    addiu $t1, $zero, 1
    sllv $t2, $t1, $v0
    or $a0, $a0, $t2
.L2700:
    addiu $v0, $v0, 1
    bne $v0, $a1, .L26e4
    addiu $v1, $v1, 4
    nor $t3, $a0, $zero
    sw $t3, 0($a2)
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    addu $v0, $zero, $zero
.end audio_generic_irq

.section .data.audio_irq_registers,"aw"
.balign 4
.globl audio_irq_registers
.type audio_irq_registers, @object
audio_irq_registers:
    .word 0x1f801070
.size audio_irq_registers, .-audio_irq_registers

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
