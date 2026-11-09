#include "bof3/bof3.h"

/* Shared readiness helper of the main executable at 0x8014DAEC, declared with
 * the eight-bit return type its own battle header gives it. This step polls it
 * twice through bare statement calls and tests only the second result, so the
 * first answer is discarded. */
u8 func_8014DAEC(void);

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. The cell is re-materialised (lui / lw) inside the taken branch, so that
 * access is a fresh load of the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F06B0
 * @behavior Entry 8 of this overlay step dispatch: the word at 0x801F09A8 is
 *           the ninth pointer of the code-pointer run based at 0x801F0988 that
 *           this target dispatcher indexes with work-object byte 1, so the
 *           table row proves the entry boundary and the no-argument ABI.
 *           From a 0x18-byte frame that keeps only $ra at 0x10 it issues the
 *           shared readiness helper at 0x8014DAEC twice, each call with a nop
 *           in its delay slot, and discards the first result. The second
 *           result is truncated to eight bits (andi 0xFF of the returned
 *           register, no mask applied to a stored value) and only when it is
 *           nonzero does the body load the scratchpad work-object pointer cell
 *           at 0x1F800044 with its own lui / lw pair and increment that
 *           object byte 2 modulo 256 (lbu / addiu 1 / sb), advancing to the
 *           next byte-2 handler step. It returns nothing through the jr $ra at
 *           0x801F06F8 and writes no other state. Beyond the dispatch
 *           indexing the role of work-object byte 2 is not established, so the
 *           offset stays anchored.
 *
 *           A byte search of every payload under out/binaries found no second
 *           copy of this 0x50-byte span, so the clone lever was declined. The
 *           sibling lift
 *           runReadinessHelperThenAdd4ToWorkByte9AndAdvanceWorkByte2At10Bmagic
 *           Magic17303_801F0648, entry 7 of the same pointer run, supplied the
 *           frame, the bare helper-call statement form, the eight-bit helper
 *           declaration and the pointer-cell reload discipline; this span has
 *           no byte-9 countdown and adds the discarded leading helper call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runReadinessHelperTwiceThenAdvanceWorkByte2WhenReadyBmagicMagic17303_801F06B0(
    void) {
  func_8014DAEC();
  if (func_8014DAEC()) {
    g_battle_work[2]++;
  }
}
