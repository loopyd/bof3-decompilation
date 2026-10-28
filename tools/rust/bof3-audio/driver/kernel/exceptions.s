# SPDX-License-Identifier: MIT
# Owned exception root. K0 is the validated current TCB; other interrupted
# registers are unchanged. The host rejects unsupported EPC/COP2 contexts and
# commits any pending load before entering. K0's TCB slot remains opaque.
# BFC0010C installs the explicit handler stack/GP and returns the captured
# ExCB base in A0. The root uses that stack's sixteen-byte O32 home area,
# without placing scanner locals there. Handler guards preserve S0-S7/GP/FP.
# BFC00110 validates T9 as a node at priority T8. BFC00114 invokes T9 with a
# saved-context guard. BFC00118 completes through the live exception-exit policy.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_exception_dispatch
.ent audio_exception_dispatch
audio_exception_dispatch:
    sw $at, 12($k0)
    sw $v0, 16($k0)
    sw $v1, 20($k0)
    sw $a0, 24($k0)
    sw $a1, 28($k0)
    sw $a2, 32($k0)
    sw $a3, 36($k0)
    sw $t0, 40($k0)
    sw $t1, 44($k0)
    sw $t2, 48($k0)
    sw $t3, 52($k0)
    sw $t4, 56($k0)
    sw $t5, 60($k0)
    sw $t6, 64($k0)
    sw $t7, 68($k0)
    sw $s0, 72($k0)
    sw $s1, 76($k0)
    sw $s2, 80($k0)
    sw $s3, 84($k0)
    sw $s4, 88($k0)
    sw $s5, 92($k0)
    sw $s6, 96($k0)
    sw $s7, 100($k0)
    sw $t8, 104($k0)
    sw $t9, 108($k0)
    sw $k1, 116($k0)
    sw $gp, 120($k0)
    sw $sp, 124($k0)
    sw $fp, 128($k0)
    sw $ra, 132($k0)
    mfc0 $t0, $14
    nop
    sw $t0, 136($k0)
    mfhi $t0
    nop
    sw $t0, 140($k0)
    mflo $t0
    nop
    sw $t0, 144($k0)
    mfc0 $t0, $12
    nop
    sw $t0, 148($k0)
    mfc0 $t0, $13
    nop
    sw $t0, 152($k0)
    lui $t0, 0xbfc0
    ori $t0, $t0, 0x010c
    jalr $t0
    nop
    addu $s3, $a0, $zero
    addiu $s4, $s3, 32
    addu $s2, $zero, $zero
.Lpriority:
    lw $s6, 0($s3)
    nop
.Lnode:
    addu $t9, $s6, $zero
    addu $t8, $s2, $zero
    lui $t0, 0xbfc0
    ori $t0, $t0, 0x0110
    jalr $t0
    nop
    beq $s6, $zero, .Lnext_priority
    nop
    lw $s1, 8($s6)
    lw $s0, 4($s6)
    beq $s1, $zero, .Lnext_node
    nop
    addu $t9, $s1, $zero
    lui $t0, 0xbfc0
    ori $t0, $t0, 0x0114
    jalr $t0
    nop
    beq $v0, $zero, .Lnext_node
    nop
    beq $s0, $zero, .Lnext_node
    addu $a0, $v0, $zero
    addu $t9, $s0, $zero
    lui $t0, 0xbfc0
    ori $t0, $t0, 0x0114
    jalr $t0
    nop
.Lnext_node:
    lw $s6, 0($s6)
    nop
    beq $zero, $zero, .Lnode
    nop
.Lnext_priority:
    addiu $s2, $s2, 1
    addiu $s3, $s3, 8
    bne $s3, $s4, .Lpriority
    nop
    lui $t0, 0xbfc0
    ori $t0, $t0, 0x0118
    jr $t0
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
