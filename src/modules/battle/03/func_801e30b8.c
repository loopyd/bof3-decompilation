/* @behavior returns `0xff` under one global mode byte, otherwise forwards a
 * selection index offset by `3` into the local picker helper.
 * @source 0x801e30b8 FUN_801e30b8
 */
__asm__(
    "\n\
    .set noreorder\n\
    .set nomacro\n\
    .text\n\
    .globl func_801e30b8\n\
    .type func_801e30b8, @function\n\
func_801e30b8:\n\
    addiu $sp,$sp,-24\n\
    lui $3,0x8014\n\
    lbu $3,25331($3)\n\
    li $2,1\n\
    bne $3,$2,8f\n\
    sw $31,16($sp)\n\
    j func_801e30e8\n\
    li $2,0xff\n\
8:\n\
    addiu $4,$4,3\n\
    jal func_801e29b4\n\
    andi $4,$4,0xff\n\
    andi $2,$2,0xff\n\
    .section .text.epi,\"ax\",@progbits\n\
    .globl func_801e30e8\n\
    .type func_801e30e8, @function\n\
func_801e30e8:\n\
    lw $31,16($sp)\n\
    addiu $sp,$sp,24\n\
    jr $31\n\
    nop\n\
");
