# SPDX-License-Identifier: MIT
# Owned supported exception vector, capture and ExCB scanner. Startup loads the
# code and data, initializes the stack-pointer cell, and links the explicit GP.
# The host preflights the supported EPC/COP2 domain, then observes the exported
# instruction boundaries without changing guest registers or charging traps.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_exception_vector
.ent audio_exception_vector
audio_exception_vector:
    lui $k0, %hi(audio_exception_dispatch)
    addiu $k0, $k0, %lo(audio_exception_dispatch)
    jr $k0
    nop
.end audio_exception_vector

.globl audio_exception_dispatch
.ent audio_exception_dispatch
audio_exception_dispatch:
    nop
    nop
    nop
    nop
    addiu $k0, $zero, 0x100
    lw $k0, 8($k0)
    nop
    lw $k0, 0($k0)
    nop
    addi $k0, $k0, 8
    sw $at, 4($k0)
    sw $v0, 8($k0)
    sw $v1, 12($k0)
    sw $ra, 124($k0)
    jal audio_exception_cause_epc
    nop
    andi $v0, $v0, 0x3c
    bne $v0, $zero, .Lsave_epc
    nop
    lw $v0, 0($v1)
    nop
    srl $v0, $v0, 24
    andi $v0, $v0, 0xfe
    addiu $at, $zero, 0x4a
    bne $v0, $at, .Lsave_epc
    nop
    addi $v1, $v1, 4
.Lsave_epc:
    sw $v1, 128($k0)
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    sw $a0, 16($k0)
    sw $a1, 20($k0)
    sw $a2, 24($k0)
    sw $a3, 28($k0)
    mfc0 $a0, $12
    nop
    sw $a0, 140($k0)
    mfc0 $a1, $13
    nop
    sw $a1, 144($k0)
    sw $k1, 108($k0)
    sw $s0, 64($k0)
    sw $s1, 68($k0)
    sw $s2, 72($k0)
    sw $s3, 76($k0)
    sw $s4, 80($k0)
    sw $s5, 84($k0)
    sw $s6, 88($k0)
    sw $s7, 92($k0)
    sw $t0, 32($k0)
    sw $t1, 36($k0)
    sw $t2, 40($k0)
    sw $t3, 44($k0)
    sw $t4, 48($k0)
    sw $t5, 52($k0)
    sw $t6, 56($k0)
    sw $t7, 60($k0)
    sw $t8, 96($k0)
    sw $t9, 100($k0)
    sw $gp, 112($k0)
    sw $sp, 116($k0)
    sw $fp, 120($k0)
    mfhi $a0
    nop
    sw $a0, 132($k0)
    mflo $a0
    nop
    sw $a0, 136($k0)
    lui $sp, %hi(audio_exception_stack_pointer)
    addiu $s3, $zero, 0x100
    lw $sp, %lo(audio_exception_stack_pointer)($sp)
    lw $s3, 0($s3)
    lui $gp, %hi(audio_exception_global_pointer)
    addiu $gp, $gp, %lo(audio_exception_global_pointer)
    addu $fp, $sp, $zero
    addi $s4, $s3, 32
.globl audio_exception_priority
audio_exception_priority:
    lw $s6, 0($s3)
    nop
    beq $s6, $zero, .Lnext_priority
    nop
.globl audio_exception_node
audio_exception_node:
    lw $s1, 8($s6)
    lw $s0, 4($s6)
    beq $s1, $zero, .Lnext_node
    nop
.globl audio_exception_verifier
audio_exception_verifier:
    jalr $s1
    nop
    beq $v0, $zero, .Lnext_node
    nop
    beq $s0, $zero, .Lnext_node
    addu $a0, $v0, $zero
.globl audio_exception_handler
audio_exception_handler:
    jalr $s0
    nop
.Lnext_node:
    lw $s6, 0($s6)
    nop
    bne $s6, $zero, audio_exception_node
    nop
.Lnext_priority:
    addi $s3, $s3, 8
    bne $s4, $s3, audio_exception_priority
    nop
.end audio_exception_dispatch

# Completed dispatch preflights the live PCB/TCB and registered jump buffer.
# Guest entry resolves them again. No stack frame or RFE belongs to this hook.
# A later explicit B0/17 restores the interrupted thread and ends dispatch.
.globl audio_exception_hook
.ent audio_exception_hook
audio_exception_hook:
    addiu $k0, $zero, 0x100
    lw $k0, 8($k0)
    lui $a0, %hi(audio_exception_hook_pointer)
    lw $k0, 0($k0)
    addiu $a0, $a0, %lo(audio_exception_hook_pointer)
    lw $a0, 0($a0)
    addiu $a1, $zero, 1
    addi $k0, $k0, 8
    lw $ra, 0($a0)
    lw $gp, 44($a0)
    lw $sp, 4($a0)
    lw $fp, 8($a0)
    lw $s0, 12($a0)
    lw $s1, 16($a0)
    lw $s2, 20($a0)
    lw $s3, 24($a0)
    lw $s4, 28($a0)
    lw $s5, 32($a0)
    lw $s6, 36($a0)
    lw $s7, 40($a0)
    addu $v0, $a1, $zero
    jr $ra
    nop
.end audio_exception_hook

.globl audio_exception_cause_epc
.ent audio_exception_cause_epc
audio_exception_cause_epc:
    mfc0 $v0, $13
    mfc0 $v1, $14
    jr $ra
    nop
.end audio_exception_cause_epc

# Entry resolves the live PCB and current TCB through the descriptor at 0x108.
# The CPU loop retains ownership until audio_exception_return_rfe retires.
# Saved K0 is opaque; K0 becomes the return PC. EPC and Cause stay unchanged.
.globl audio_exception_return
.ent audio_exception_return
audio_exception_return:
    addiu $k0, $zero, 0x100
    lw $k0, 8($k0)
    nop
    lw $k0, 0($k0)
    nop
    addi $a0, $k0, 8
    lw $v0, 0x88($a0)
    nop
    mtlo $v0
    nop
    lw $v0, 0x84($a0)
    nop
    mthi $v0
    nop
    lw $v0, 0x8c($a0)
    nop
    mtc0 $v0, $12
    nop
    lw $v0, 0x08($a0)
    lw $v1, 0x0c($a0)
    lw $a1, 0x14($a0)
    lw $a2, 0x18($a0)
    lw $a3, 0x1c($a0)
    lw $t0, 0x20($a0)
    lw $t1, 0x24($a0)
    lw $t2, 0x28($a0)
    lw $t3, 0x2c($a0)
    lw $t4, 0x30($a0)
    lw $t5, 0x34($a0)
    lw $t6, 0x38($a0)
    lw $t7, 0x3c($a0)
    lw $s0, 0x40($a0)
    lw $s1, 0x44($a0)
    lw $s2, 0x48($a0)
    lw $s3, 0x4c($a0)
    lw $s4, 0x50($a0)
    lw $s5, 0x54($a0)
    lw $s6, 0x58($a0)
    lw $s7, 0x5c($a0)
    lw $t8, 0x60($a0)
    lw $t9, 0x64($a0)
    lw $k1, 0x6c($a0)
    lw $gp, 0x70($a0)
    lw $sp, 0x74($a0)
    lw $fp, 0x78($a0)
    lw $ra, 0x7c($a0)
    nop
    lw $at, 0x04($a0)
    lw $k0, 0x80($a0)
    nop
    lw $a0, 0x10($a0)
    nop
    jr $k0
.globl audio_exception_return_rfe
audio_exception_return_rfe:
    .word 0x42000010
.end audio_exception_return

.section .data.audio_exception_hook,"aw"
.balign 4
.globl audio_exception_hook_pointer
audio_exception_hook_pointer:
    .word 0

.section .data.audio_exception_stack,"aw"
.balign 4
.globl audio_exception_stack_pointer
audio_exception_stack_pointer:
    .word 0
