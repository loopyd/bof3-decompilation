.set noreorder
.section .text.start,"ax",@progbits
.globl _start
.ent _start
_start:
    lui $sp, 0x8009
    ori $sp, $sp, 0x6000
    move $gp, $zero
    la $t0, __bss_start
    la $t1, __bss_end
1:
    beq $t0, $t1, 2f
    nop
    sw $zero, 0($t0)
    b 1b
    addiu $t0, $t0, 4
2:
    jal player_main
    nop
3:
    b 3b
    nop
.end _start
