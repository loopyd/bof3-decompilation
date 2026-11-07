#include "bof3/bof3.h"

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer for each of the four access groups below. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void func_801F38DC(void);

/* @source 0x801F37A0
 * @behavior Overlay work-record step handler, entry 1 of the byte-1 handler
 *           table based at 0x801F5948 whose word at 0x801F594C holds this
 *           address; the run's entry 0 is 0x801F3718 and entry 2 is
 *           0x801F3860. This target's dispatcher
 *           dispatchStateByte1Table5948World02Area10313_801F36D4 indexes that
 *           table with the work record's state byte at +0x01. The body reads
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and, in one pointer group, advances its signed word at
 *           +0x1C by 0x01000000 and clamps that word to the word at +0x14 plus
 *           0x08000000, so the interpolated +0x1C value cannot pass its limit.
 *           It then reloads the cell, sweeps the halfword at +0x20 down by
 *           0x200 with that store in the call delay slot of the overlay
 *           helper 0x801F38DC, reloads the cell again, and decrements the
 *           countdown byte at +0x09 modulo 256. When the countdown reads zero
 *           it re-arms that byte with 0x10 through a fresh cell read and
 *           advances the record's byte at +0x01 by one, so the next frame
 *           dispatches the handler at 0x801F3860, which continues the same
 *           +0x20 sweep. Takes no arguments and returns nothing; its
 *           0x18-byte frame only holds $ra at 0x10 across the call. The roles
 *           of the record's words at +0x14 and +0x1C, of its halfword at
 *           +0x20 and of the 0x01000000 step stay described by offset.
 *
 * Name evidence: the only in-image reference to 0x801F37A0 is the word at
 * 0x801F594C, the second entry of the local pointer run based at 0x801F5948
 * that this target's exact
 * dispatchStateByte1Table5948World02Area10313_801F36D4 indexes with the work
 * record's byte at +0x01; there is no jal caller, which is the dispatch shape
 * rather than missing evidence. The advance-and-clamp, countdown re-arm and
 * byte-1 advance roles come from this body's own immediates and stores.
 *
 * Shape evidence: the overlays world03/area134 at 0x801F4400 and
 * world03/area132 at 0x801F312C hold copies of these 0xC0 bytes that differ in
 * exactly the two immediate bytes of the jal at 0x801F37F4 (0x801F38DC here,
 * 0x801F4B6C and 0x801F3268 in their own payloads), compared byte for byte
 * against this target's payload. A discriminating immediate that differs is a
 * relocation-different sibling, so it is shape evidence only and never clone
 * eligibility. The statement order, pointer-cell reload points and u8
 * accumulator spelling of the countdown group follow this target's exact
 * drawWorkRampStepAndAdvanceWorkByte2At19World02Area10313_801F507C and the
 * world03/area134 lift of the same body; this function's own immediates, cell
 * offsets, sweep value and call target come from its original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 * The pointer cell is declared plain, not `volatile`: every reload this body
 * shows is already forced by an intervening store through the published pointer
 * or by the helper call, and the plain cell leaves the re-arm constant free to
 * fill the countdown test's delay slot, which the world03/area134 lift of the
 * same bytes could not reach through its volatile cell.
 */
void advanceWorkWord1CClampedToWord14AndRearmCountdownThenAdvanceWorkByte1World02Area10313_801F37A0(
    void) {
  u8* work;
  u8* sweep;
  u8* cursor;
  s32 next;
  s32 limit;
  u8 countdown;

  work = D_1F800044;
  next = FIELD_REF(s32, work, 0x1C) + 0x1000000;
  limit = FIELD_REF(s32, work, 0x14) + 0x8000000;
  FIELD_REF(s32, work, 0x1C) = next;
  if (limit < next) {
    FIELD_REF(s32, work, 0x1C) = limit;
  }
  sweep = D_1F800044;
  FIELD_REF(u16, sweep, 0x20) -= 0x200;
  func_801F38DC();
  cursor = D_1F800044;
  countdown = (u8)(cursor[9] - 1);
  cursor[9] = countdown;
  if (countdown == 0) {
    D_1F800044[9] = 0x10;
    D_1F800044[1]++;
  }
}
