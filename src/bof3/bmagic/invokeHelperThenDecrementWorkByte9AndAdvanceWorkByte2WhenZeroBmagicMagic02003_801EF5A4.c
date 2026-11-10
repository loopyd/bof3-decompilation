#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, spelled through this address-derived
 * symbol, the same named cell the exact lifts of this target already use. A
 * named cell symbol lets the assembler fold the constant address into each load
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)), and the original re-reads the cell for
 * the trailing increment because the byte store through the pointer it supplied
 * may alias the cell object. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared readiness helper of the main executable, outside this overlay's image.
 * This target's generated undefined-funcs map binds it at 0x8014DAEC, and the
 * exact sibling setWorkByte9To8AndAdvanceWorkByte2WhenHelperReadyBmagic
 * Magic01703_801EF694 declares the same unsigned byte return. This call site
 * discards the answer, which is why the original issues the jal with a nop in
 * its delay slot and never tests $v0. */
extern u8 func_8014DAEC(void);

/* @source 0x801EF5A4
 * @behavior Entry 8 of the code-pointer run at 0x801EEC04..0x801EEC47, the
 *           leading data block of this overlay: the word at 0x801EEC24 holds
 *           this function's address and is the only in-image reference to it,
 *           and no jal in this payload names it. The overlay entry's dispatcher
 *           dispatchWorkByte1HandlerBmagicMagic02003_801EEC48 copies only the
 *           run's first six entries into its own frame, so no instruction of
 *           this payload reads entry 8 and the word is the span's only incoming
 *           path that the payload itself carries. The body first invokes the
 *           shared readiness helper func_8014DAEC with no arguments and
 *           discards the returned byte (jal with a nop delay slot, no test on
 *           $v0 afterwards). It then materializes the work-object pointer cell
 *           at 0x1F800044 (lui $v1,0x1F80 / lw $v1,0x44($v1)), decrements that
 *           object's unsigned byte 9 in place (lbu 9 / addiu -1 / sb 9) and
 *           tests the stored byte truncated to eight bits (andi $v0,$v0,0xFF /
 *           bnez), so the guarded arm runs only when that countdown reaches
 *           zero. On zero it re-reads the 0x1F800044 cell -- a store through
 *           the loaded pointer may republish it -- and increments work byte 2
 *           at byte width (lbu 2 / addiu 1 / sb 2). The 0x18-byte frame keeps
 *           only $ra at 0x10, every delay slot in the span is nop, and the
 *           function takes no arguments, returns no value and reads or writes
 *           no other state. Roles of work byte 9 and work byte 2 are not
 *           established by this span, so they stay offset-anchored. Statement
 *           order, the in-place countdown and the cell re-materialization
 *           grouping follow this target's exact sibling
 *           drawFadeTileThenDecrementWorkByte9AndAdvanceWorkByte1WhenWorkByteB
 *           IsZeroBmagicMagic02003_801EF234. A byte search over every payload
 *           under out/binaries found exactly one copy of this 0x64-byte span, so
 *           the clone lever was declined. The span opens with addiu $sp,-0x18
 *           and closes in jr $ra through its own frame, so the data lever did
 *           not apply, and it sits between the reviewed boundaries at
 *           0x801EF518, whose span ends at this address, and 0x801EF608, so the
 *           carve lever did not apply either.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperThenDecrementWorkByte9AndAdvanceWorkByte2WhenZeroBmagicMagic02003_801EF5A4(
    void) {
  u8 *work;
  u8 countdown;
  u8 *object;

  func_8014DAEC();
  work = D_1F800044;
  countdown = work[9] - 1;
  work[9] = countdown;
  if (countdown == 0) {
    object = D_1F800044;
    object[2] += 1;
  }
}
