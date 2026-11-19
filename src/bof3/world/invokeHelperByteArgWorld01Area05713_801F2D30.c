#include "bof3/bof3.h"

extern u8 D_80145029;

void func_80161D20(s32 arg0);

/* @source 0x801F2D30
 * @behavior Thin overlay wrapper: passes the shared main-RAM byte D_80145029,
 * loaded unsigned, as the single argument of the shared EXE helper at
 * 0x80161D20 and returns. The load is materialised before the stack frame
 * because the argument is its only consumer: `lui $a0,%hi` plus the folded
 * `lbu $a0,%lo($a0)` come first, then `addiu $sp,$sp,-0x18` and
 * `sw $ra,0x10($sp)`. The single call keeps its `nop` delay slot and the
 * epilogue restores the 0x18-byte frame (`lw $ra`, `addiu $sp`, `jr $ra`,
 * `nop`). Takes no arguments and returns nothing.
 *
 * The 0x28-byte payload span compares byte-identical to the exact spans at
 * 0x801F2E9C (world01/area039/13) and 0x801F4EBC (world02/area108/14), which
 * hand the same shared byte to the same shared helper, so the body is a
 * verified twin and not a near-match. The cell and the callee both sit in the
 * shared main executable, outside this overlay's payload, and keep the symbols
 * those siblings already bind. No `jal` in this payload reaches this span;
 * instead one pointer word in the overlay's own data blob (payload offset
 * 0x16A4, vram 0x801F42A4) names it, in the same run that names 0x801F2D58 and
 * 0x801F2C04, so the caller is the script blob and the parameter roles stay
 * structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperByteArgWorld01Area05713_801F2D30(void) {
  func_80161D20(D_80145029);
}
