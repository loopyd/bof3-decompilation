#include "bof3/bof3.h"

/* Work record the main program publishes through the scratchpad pointer cell
 * 0x1F800044. The original re-materialises the cell before each store below,
 * emitting its own `lui`/`lw` pair per access, so the source names the cell
 * extern and evaluates it again at every store. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-program helper of the boss overlays; the caller below masks the
 * returned word down to one byte, so the declared result is the wider word the
 * original masks. Its role is not established here. */
s32 func_8014D978(void); /* @source 0x8014D978 */

/* @source 0x800C1940
 * @behavior Entry 0 of the byte-3 handler run this overlay bases at 0x800C2714,
 * the word 0x800C2714 inside its reviewed `textbin` row stateStepDataTable:
 * callStateTable2714Boss scales the work record's
 * unsigned byte 3 by four and calls that entry with no arguments, and the run's
 * own void-returning type is what this body returns. No jal in the payload
 * reaches 0x800C1940, so the run is its only caller and the argument roles stay
 * structural. It calls the shared main-program helper func_8014D978 with no
 * arguments and keeps the low byte of the returned word; while that byte is
 * nonzero it writes 0x40 into work byte 9, 0x20 into work byte 0xA and 1 into
 * work byte 3 - the state index the byte-3 dispatcher reads, so this store moves
 * the next dispatch to entry 1 of the same run, 0x800C199C - through the
 * scratchpad pointer cell 0x1F800044, and writes nothing when the masked byte is
 * zero. The 0x18-byte frame only keeps $ra at 0x10 across the call. The 0x40 and
 * 0x20 values reach their stores from the branch delay slot and from the second
 * cell load's delay slot because nothing is live in $v0 past the handler return;
 * the first and third stores keep the original nop in that slot. Roles of work
 * bytes 9 and 0xA are not established here: the sibling
 * seedQuadSizeAndAnchorThenDrawRotatedQuadAndAdvanceStateBossBoss03416_800C1A78
 * passes work byte 9 to renderSpinQuad, which this lane did not lift, so the name
 * describes the recorded effect.
 *
 * Shape: the 0x3C-byte exact lift
 * setStateByte3To1WhenHelperReturnsNonzeroBossBoss01216_800C193C of
 * BIN/BOSS/BOSS012.EMI#16 holds this span's prologue, its jal, its
 * mask-and-test and its final state store statement for statement, and supplied
 * the statement order, the declaration style and the delay-slot form; this span
 * adds the two earlier byte stores ahead of that state store, and its own
 * original bytes supply every literal, offset and store order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateHelperReturnsNonzero(
    void) {
  if (func_8014D978() & 0xFF) {
    D_1F800044[9] = 0x40u;
    D_1F800044[10] = 0x20u;
    D_1F800044[3] = 1u;
  }
}
