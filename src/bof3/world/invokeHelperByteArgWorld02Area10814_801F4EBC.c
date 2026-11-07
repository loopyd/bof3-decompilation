#include "bof3/bof3.h"

/* Shared main-RAM byte at 0x80145029. This overlay's own bytes prove only that
 * the function reads it unsigned (`lbu`) and hands it straight to the helper;
 * the value's role is the shared EXE selection index byte that other overlays
 * document, which this lane does not re-verify here. Splat resolves the address
 * through this target's generated undefined-symbol map. */
extern u8 D_80145029; /* @source 0x80145029 @kind unknown */

/* Shared EXE helper at 0x80161D20, outside this overlay's payload and still
 * unlifted, so only its prototype is declared. It consumes the byte in $a0 and
 * its result is discarded. */
void func_80161D20(s32 arg0);

/* @source 0x801F4EBC
 * @behavior Thin overlay wrapper: passes the shared main-RAM byte D_80145029,
 * loaded unsigned, as the single argument of the shared helper at 0x80161D20
 * and returns. The load is materialised before the stack frame because the
 * argument is its only consumer: `lui $a0,%hi` plus the folded
 * `lbu $a0,%lo($a0)` come first, then `addiu $sp,$sp,-0x18` and
 * `sw $ra,0x10($sp)`. The single call keeps its `nop` delay slot and the
 * epilogue restores the 0x18-byte frame (`lw $ra`, `addiu $sp`, `jr $ra`,
 * `nop`). Takes no arguments and returns nothing. Byte-identical to the exact
 * siblings at 0x801F434C (world03/area145/14) and 0x801F2E9C
 * (world01/area039/13).
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperByteArgWorld02Area10814_801F4EBC(void) {
  func_80161D20(D_80145029);
}
