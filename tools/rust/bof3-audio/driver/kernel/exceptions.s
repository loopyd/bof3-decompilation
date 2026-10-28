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

# Completed guest dispatch supplies the preflighted live jump buffer in A0
# and one in A1. No stack frame, RFE or TCB restoration belongs to this hook.
# A later explicit B0/17 restores the interrupted thread and ends dispatch.
.globl audio_exception_hook
.ent audio_exception_hook
audio_exception_hook:
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
