# SPDX-License-Identifier: MIT
# Owned diagnostic ABI. Guest writes reach the PCSX-Redux debug-character port.
# Entry guards validate strings, formats and the printf frame before execution.
.set noreorder
.set noat
.section .text
.balign 4

.globl audio_diagnostic_puts
.ent audio_diagnostic_puts
audio_diagnostic_puts:
    lui $t0, 0x1f80
    ori $t0, $t0, 0x2080
    addu $t1, $a0, $zero
.Lputs_next:
    lbu $t2, 0($t1)
    addiu $t1, $t1, 1
    beq $t2, $zero, .Lputs_return
    nop
    sb $t2, 0($t0)
    b .Lputs_next
    nop
.Lputs_return:
    jr $ra
    nop
.end audio_diagnostic_puts

# o32 arguments begin in A1..A3 and continue at the caller's SP+16.
# The return counts expanded fields and %% bytes, as the owned formatter does.
.globl audio_diagnostic_printf
.ent audio_diagnostic_printf
audio_diagnostic_printf:
    nop
    sw $a1, 4($sp)
    sw $a2, 8($sp)
    sw $a3, 12($sp)
    addiu $sp, $sp, -96
    sw $s0, 48($sp)
    sw $s1, 52($sp)
    sw $s2, 56($sp)
    sw $s3, 60($sp)
    sw $ra, 64($sp)
    addu $s0, $a0, $zero
    addiu $s1, $sp, 100
    addu $s2, $zero, $zero
    lui $s3, 0x1f80
    beq $s0, $zero, .Lprintf_return
    ori $s3, $s3, 0x2080
.Lprintf_next:
    lbu $t0, 0($s0)
    addiu $s0, $s0, 1
    beq $t0, $zero, .Lprintf_return
    addiu $t1, $zero, 0x25
    beq $t0, $t1, .Lprintf_conversion
    nop
    sb $t0, 0($s3)
    b .Lprintf_next
    nop
.Lprintf_conversion:
    lbu $t0, 0($s0)
    addiu $s0, $s0, 1
    bne $t0, $t1, .Lprintf_argument
    nop
    sb $t0, 0($s3)
    b .Lprintf_next
    addiu $s2, $s2, 1
.Lprintf_argument:
    lw $t2, 0($s1)
    addiu $s1, $s1, 4
    addiu $t1, $zero, 0x73
    beq $t0, $t1, .Lprintf_string
    addiu $t1, $zero, 0x64
    beq $t0, $t1, .Lprintf_decimal
    nop
    # The validated remaining conversion is 02x, 04x or 08x.
    lbu $t4, 0($s0)
    addiu $s0, $s0, 2
    addiu $t4, $t4, -0x30
    b .Lprintf_number
    addiu $t1, $zero, 16
.Lprintf_decimal:
    addu $t4, $zero, $zero
    bgez $t2, .Lprintf_number
    addiu $t1, $zero, 10
    addiu $t0, $zero, 0x2d
    sb $t0, 0($s3)
    subu $t2, $zero, $t2
    addiu $s2, $s2, 1
.Lprintf_number:
    addiu $t5, $sp, 48
    addu $t6, $zero, $zero
.Lprintf_digit:
    divu $zero, $t2, $t1
    mfhi $t0
    mflo $t2
    sltiu $t3, $t0, 10
    bne $t3, $zero, .Lprintf_store_digit
    addiu $t0, $t0, 0x30
    addiu $t0, $t0, 0x27
.Lprintf_store_digit:
    addiu $t5, $t5, -1
    sb $t0, 0($t5)
    bne $t2, $zero, .Lprintf_digit
    addiu $t6, $t6, 1
.Lprintf_padding:
    sltu $t0, $t6, $t4
    beq $t0, $zero, .Lprintf_number_output
    addiu $t0, $zero, 0x30
    addiu $t5, $t5, -1
    sb $t0, 0($t5)
    b .Lprintf_padding
    addiu $t6, $t6, 1
.Lprintf_number_output:
    addu $s2, $s2, $t6
.Lprintf_emit_digit:
    lbu $t0, 0($t5)
    addiu $t5, $t5, 1
    sb $t0, 0($s3)
    addiu $t6, $t6, -1
    bne $t6, $zero, .Lprintf_emit_digit
    nop
    b .Lprintf_next
    nop
.Lprintf_string:
    bne $t2, $zero, .Lprintf_string_next
    nop
    lui $t2, %hi(.Lprintf_null)
    addiu $t2, $t2, %lo(.Lprintf_null)
.Lprintf_string_next:
    lbu $t0, 0($t2)
    addiu $t2, $t2, 1
    beq $t0, $zero, .Lprintf_next
    nop
    sb $t0, 0($s3)
    b .Lprintf_string_next
    addiu $s2, $s2, 1
.Lprintf_return:
    addu $v0, $s2, $zero
    lw $s0, 48($sp)
    lw $s1, 52($sp)
    lw $s2, 56($sp)
    lw $s3, 60($sp)
    lw $ra, 64($sp)
    addiu $sp, $sp, 96
    jr $ra
    nop
.end audio_diagnostic_printf

.section .rodata.audio_diagnostic,"a"
.Lprintf_null:
    .asciz "(null)"
