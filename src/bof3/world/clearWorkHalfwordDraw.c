#include "bof3/bof3.h"

/* Draw routine of this overlay, a lifted `c` span of this target at 0x801F4640.
 * Its argument is stored once and tested with `beqz`, so a non-zero value
 * enables the second rendering pass; this handler and the sibling entry at
 * 0x801F45E4 pass zero. */
void renderPassTexturedDepth(
    s32 variant);

/* @source 0x801F453C
 * @behavior Entry 0 of the three-entry byte-2 handler table based at
 *           0x801F58BC, the table the overlay dispatcher at 0x801F44F8
 *           indexes by the state byte at offset 0x02 of the work record
 *           published at the scratchpad pointer cell 0x1F800044. Reads that
 *           state byte, stores zero into the record halfword at 0x30 - the
 *           progress value the overlay draw routine at 0x801F4640 uses as its
 *           quad's horizontal origin, and the value the sibling entries
 *           advance and decrement - and then stores the state byte advanced by
 *           one, so the next dispatch takes entry 1 at 0x801F4574. The draw
 *           routine then runs with argument 0, the variant flag whose zero
 *           keeps its second rendering pass off. Takes no arguments and
 *           returns nothing; the 0x18-byte frame only keeps $ra at 0x10.
 *
 * Name evidence: the payload word at 0x801F58BC names this span as the first
 * of three handlers that the byte-2 dispatcher at 0x801F44F8 selects, so the
 * body's job is the record reset that precedes the animated entries
 * 0x801F4574 and 0x801F45E4, both of which keep the same byte-2 advance and
 * then drive halfword 0x30 of the same record. No flag, camera or cue test
 * stands between the record read and the draw call, so the name carries no
 * gate clause.
 *
 * Shape evidence: sibling 0x801F4574 of the same table is an exact lift and
 * runs the identical statement order - record cell load, byte-2 read, the
 * record write and the advance - with a flag gate, a sound cue and the variant
 * flag 1 around it, so it supplied the record accessor spelling, the local
 * that saves the byte before it is overwritten and the callee declaration.
 * Sibling 0x801F45E4 adds 0x20 to the same halfword and tests it against 0x100
 * before the same draw call, which fixes halfword 0x30 as the shared progress
 * counter. Neither sibling is clone-eligible for this span: both differ in
 * body, not only in relocation words. A scan of every payload under
 * out/binaries for this span with its `jal` word masked found only its own
 * copy at payload offset 0x193C, so no byte-identical sibling exists in the
 * shipped corpus and no clone or carve lever applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkHalfwordDraw(
    void) {
  u8* work;
  u8 step;

  /* The record's state byte is read before the progress halfword is cleared,
   * because the original keeps that loaded byte live across the halfword
   * store and adds to it after the store. */
  work = *(u8**)SPAD_ADDR(u8, 0x44u);
  step = work[2];
  FIELD_REF(u16, work, 0x30) = 0;
  work[2] = step + 1;
  renderPassTexturedDepth(
      0);
}
