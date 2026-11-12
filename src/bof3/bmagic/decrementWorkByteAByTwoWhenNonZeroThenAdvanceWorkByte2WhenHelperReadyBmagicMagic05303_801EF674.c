#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * Every access here is byte-addressed, so the byte view is all this body needs,
 * and the name is the one this target's exact lift
 * decrementGlobalWorkByteBAndInvokeHelperWhenWorkByte4IsZero
 * BmagicMagic05303_801EF6D8 already uses for the same cell. The original
 * materializes the cell once (lui $v1,0x1F80 / lw $v1,0x44($v1)) before the
 * frame is created and reads and writes work byte 0x0A through that one value,
 * so that guard is a single access group with no store in between; the second
 * group sits past the helper call, whose jal clobbers $v1, so the compiler
 * reloads the cell there exactly as the original does. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared readiness helper in the main executable, outside this overlay's
 * image. It reports a readiness byte: the original zero-extends the returned
 * register with andi 0xFF before testing it, which is the u8 return the exact
 * lifts of this helper already declare. This target's generated
 * undefined_funcs_auto.txt binds the address through the remaining assembly
 * spans, so no target-map row is needed. */
u8 func_8014DAEC(void);

/* @source 0x801EF674
 * @behavior Handler at entry index 4 of the ten-pointer code run
 *           0x801F0550..0x801F0574 (0x801EF0A8, 0x801EF174, 0x801EF420,
 *           0x801EF5C0, 0x801EF674, 0x801EF6D8, 0x801EFD94, 0x801EFE1C,
 *           0x801EFF24, 0x801F0018). That table's only in-payload reader is the
 *           dispatcher dispatchStateByte1Table0550BmagicMagic05303_801EF064,
 *           which indexes the run with the work record's unsigned byte +1 and
 *           calls the selected cell as void(void), so this body is invoked with
 *           no arguments and returns nothing. It reads the scratchpad
 *           work-object pointer cell at 0x1F800044 once, before the 0x18-byte
 *           frame is created, and tests the work object's unsigned byte 0x0A
 *           with lbu and beqz; only when that byte is non-zero does it subtract
 *           two at byte width (lbu 0x0A / addiu -2 in the branch delay slot /
 *           sb 0x0A). It then invokes the shared readiness helper
 *           func_8014DAEC, zero-extends the returned byte with andi 0xFF and
 *           tests it; only when the helper reports ready does it re-read the
 *           pointer cell, now past the jal that clobbered its register, and
 *           increment the work object's dispatch byte 0x02 (lbu 0x02 / addiu 1
 *           / sb 0x02), advancing the dispatcher to table entry 5
 *           (0x801EF6D8). The frame holds only $ra at 0x10, the body returns
 *           through the jr $ra at 0x801EF6D0, and it reads or writes no other
 *           state. The roles of work byte 0x0A and of work byte 0x02 beyond the
 *           dispatcher's index are not established, so both stay
 *           offset-anchored. The 0x64-byte span has no byte-identical window in
 *           any payload under out/binaries (only its own occurrence at payload
 *           offset 2676), so the clone lever offered no source. The statement
 *           order, operand forms, offset spelling and declaration style follow
 *           the exact same-target sibling
 *           decrementGlobalWorkByteBAndInvokeHelperWhenWorkByte4IsZero
 *           BmagicMagic05303_801EF6D8 (cell load before frame creation, lbu
 *           guard without andi, byte-width work-byte update) and the exact
 *           advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C (u8
 *           readiness-helper declaration, andi 0xFF test and the post-call
 *           reload of the pointer cell followed by the byte-0x02 increment).
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAByTwoWhenNonZeroThenAdvanceWorkByte2WhenHelperReadyBmagicMagic05303_801EF674(void) {
  if (g_battle_work[0x0A] != 0) {
    g_battle_work[0x0A] -= 2;
  }
  if (func_8014DAEC() != 0u) {
    g_battle_work[0x02]++;
  }
}
