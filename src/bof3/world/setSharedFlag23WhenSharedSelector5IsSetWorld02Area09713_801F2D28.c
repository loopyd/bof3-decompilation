#include "bof3/bof3.h"

/* Base of the shared main-RAM flag bank the EXE-side setter below indexes:
 * this body materializes it as `lui $a0,0x8014` / `addiu $a0,$a0,0x4F28`. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind unknown */

u32 func_80166140(u32 arg0);
void func_8015B580(void* arg0, s32 arg1);

/* @source 0x801F2D28
 * @behavior Overlay step reached only from the loader table, so it has no `jal`
 *           caller. It asks the main-exe byte query func_80166140 for selector 5
 *           and, when that answer is nonzero, sets flag 0x23 of the shared
 *           main-RAM flag bank D_80144F28 through the EXE-side flag setter
 *           func_8015B580; a zero answer skips the setter. Every path returns 0,
 *           so the step reports "not complete" whether or not it armed the flag.
 *           It reads no other state; the 0x18-byte frame only keeps $ra at 0x10
 *           across the two calls.
 *
 * Name evidence: the query argument is the `addiu $a0,$zero,0x5` delay slot of
 * the `jal 0x80166140`; the setter call materializes the bank as the
 * `lui/addiu $a0,%hi/%lo(D_80144F28)` pair with `addiu $a1,$zero,0x23` in its
 * delay slot, and `beqz $v0` on the query result skips it, so 0x23 is the flag
 * index this body arms. The helper roles are the ones the already-lifted
 * siblings in this tree record (func_80166140 a main-exe byte-result query
 * taking a selector id, func_8015B580 the setter of one flag of a byte bank at
 * the address it is handed). No role for selector 5 or for flag 0x23 is
 * evidenced beyond those two uses, so the name records the writes rather than
 * inventing a game-state role.
 *
 * Shape evidence: statement order, operand forms and declaration style are
 * taken from the same target's sibling
 * audioMapRangeGate
 * (0x801F2C98) for the func_80166140(5) query and the zero return after it, and
 * from the in-tree setter callers setSharedFlagBit4World02Area07714_801F5BD8
 * and setFlag6eThenSetFlag71WhenFlags6dTo70SetWorld01Area05613_801F346C for the
 * `func_8015B580(&D_80144F28, <flag>)` form. Those are shape evidence rather
 * than clone sources: a scan of the 398 payloads under out/binaries finds this
 * 56-byte span only in this payload, so the clone lever does not apply.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setSharedFlag23WhenSharedSelector5IsSetWorld02Area09713_801F2D28(void) {
  if (func_80166140(5) != 0) {
    func_8015B580(&D_80144F28, 0x23);
  }
  return 0;
}
