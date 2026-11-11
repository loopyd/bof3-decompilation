# SPDX-License-Identifier: MIT
# Position-independent reset block copied to the beginning of the owned ROM.
# The linked image follows at audio_reset_payload. No incoming RAM is needed.
.set noreorder
.set noat
.section .text.audio_reset,"ax"
.balign 4
.globl audio_reset_start
.ent audio_reset_start
audio_reset_start:
    lui $t0, 0xfffe
    ori $t0, $t0, 0x0130
    lui $t1, 0x0001
    ori $t1, $t1, 0xe988
    sw $t1, 0($t0)
    mtc0 $zero, $12
    nop
    nop
    lui $t0, 0x1f80
    ori $t0, $t0, 0x1014
    lui $t1, 0x2009
    ori $t1, $t1, 0x31e1
    sw $t1, 0($t0)
    lui $t0, %hi(audio_reset_payload)
    addiu $t0, $t0, %lo(audio_reset_payload)
    lui $t1, %hi(audio_kernel_image)
    addiu $t1, $t1, %lo(audio_kernel_image)
    lui $t2, %hi(audio_kernel_image_end)
    addiu $t2, $t2, %lo(audio_kernel_image_end)
.Lload_image:
    lw $t3, 0($t0)
    addiu $t0, $t0, 4
    sw $t3, 0($t1)
    addiu $t1, $t1, 4
    bne $t1, $t2, .Lload_image
    nop
    lui $t9, %hi(audio_calls_start)
    addiu $t9, $t9, %lo(audio_calls_start)
    jr $t9
    nop
.globl audio_reset_end
audio_reset_end:
.end audio_reset_start
