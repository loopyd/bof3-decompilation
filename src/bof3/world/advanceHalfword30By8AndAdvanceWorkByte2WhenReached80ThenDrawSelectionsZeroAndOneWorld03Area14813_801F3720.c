#include "bof3/bof3.h"

/* Overlay drawing routine of this payload, lifted at 0x801F37B8: three
 * selection rows of textured quads built from packed vertex triples, taking the
 * selection index and a signed byte mode. */
void drawThreeSelectionQuadsFromVertexTriplesScaledByHalfword30World03Area14813_801F37B8(
    s32 arg0, s32 arg1);

/* @source 0x801F3720
 * @behavior Handler at word 2 of the overlay state-byte-2 dispatch run based at
 *           0x801F5024, so it has no jal caller: the dispatcher at 0x801F35D8
 *           reaches it through that table. Reads the work record pointer from
 *           the scratchpad cell 0x1F800044 once, raises the record halfword at
 *           +0x30 by 8 and stores it back, then advances the record dispatch
 *           byte at +2 by one when the signed halfword has reached 0x80.
 *           Afterwards it calls the overlay quad drawing routine at 0x801F37B8
 *           twice: selection 0 with signed byte mode -1, then selection 1 with
 *           mode 1. Takes no arguments and returns nothing; its 0x18-byte frame
 *           only holds ra at +0x10 and the record pointer load precedes the
 *           frame creation.
 *
 * Name evidence: the quad routine scales the record halfword at +0x30 by the
 * signed byte row selected by its first argument and adds it to its own vertex
 * table, so +0x30 is the panel offset this handler steps. The record byte at +2
 * is the index the overlay dispatcher reads (dispatchStateByte2Table5024...),
 * which makes the single store to it an advance of that dispatch state. The two
 * draw calls carry the selection indexes and modes the bytes show, in order.
 *
 * Shape evidence: a little-endian masked scan of every payload under
 * out/binaries (call-target and internal branch words masked) finds this
 * 104-byte body only at 0x801F3720, so no clone representative exists. The word
 * neighbours in the same dispatch run differ in body: word 1 (0x801F36A0) gates
 * on a helper and draws with mode 1 twice, and word 3 (0x801F3788) writes the
 * shared overlay byte instead of stepping the halfword.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By8AndAdvanceWorkByte2WhenReached80ThenDrawSelectionsZeroAndOneWorld03Area14813_801F3720(
    void) {
  u8* work;
  s32 offset;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  offset = FIELD_REF(u16, work, 0x30) + 8;
  FIELD_REF(u16, work, 0x30) = offset;
  if ((s16)offset >= 0x80) {
    work[2]++;
  }
  drawThreeSelectionQuadsFromVertexTriplesScaledByHalfword30World03Area14813_801F37B8(
      0, -1);
  drawThreeSelectionQuadsFromVertexTriplesScaledByHalfword30World03Area14813_801F37B8(
      1, 1);
}
