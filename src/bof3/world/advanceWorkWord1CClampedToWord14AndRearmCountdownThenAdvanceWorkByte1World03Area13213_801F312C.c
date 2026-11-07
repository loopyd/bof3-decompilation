#include "bof3/bof3.h"

void drawTexturedRibbonAlongWorkSpanWorld03Area13213_801F3268(void);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer for each of the four access groups below.
 * Declared plain, not volatile: every reload this body shows is already forced
 * by the published store or by the helper call between the groups, and the
 * plain cell leaves the re-arm constant free to fill the countdown test's
 * delay slot. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F312C
 * @behavior Overlay work-record step handler, entry 1 of the byte-1 handler
 *           table based at 0x801F3E00, whose three words hold 0x801F30C0,
 *           this address at 0x801F3E04 and 0x801F31EC. The table is indexed
 *           with the work record's state byte at +1 by this target's exact
 *           dispatcher dispatchStateByte1Table3e00WorldWorld03Area13213_801F307C.
 *           Entry 0 seeds the fields this body moves: it arms the countdown
 *           byte at +0x09 with 0x40, clears the halfword at +0x20, stores
 *           0x01000000 in the word at +0x24, copies the saved words at +0x34,
 *           +0x38 and +0x3C into the record's words at +0x0C, +0x10 and +0x14
 *           while also starting the word at +0x1C from that +0x3C copy, and
 *           then advances the state byte at +0x01 and calls func_8015DF18 with
 *           0x207.
 *           This handler then advances the record's signed word at +0x1C by
 *           the same 0x01000000 step and clamps it to the word at +0x14 plus
 *           0x08000000 (eight steps beyond it), so the interpolated +0x1C
 *           value cannot pass its limit; sweeps the halfword at +0x20 down by
 *           0x200; runs the overlay helper at 0x801F3268, which builds its own
 *           frame pointer in $a0, takes no arguments and returns nothing; and
 *           decrements the countdown byte at +0x09 modulo 256. When that
 *           countdown reads zero it re-arms the byte with 0x10 and advances the
 *           record's state byte at +0x01 by one, so the next frame dispatches
 *           entry 2 at 0x801F31EC. Takes no arguments and returns nothing; its
 *           0x18-byte frame only holds $ra at 0x10 across the call, whose delay
 *           slot keeps the +0x20 halfword store. The roles of the record's
 *           words at +0x14 and +0x1C, of its halfword at +0x20 and of the
 *           0x01000000 step stay described by offset.
 *
 * Name evidence: the only in-image reference to 0x801F312C is the word at
 * 0x801F3E04, entry 1 of the pointer run based at 0x801F3E00; there is no jal
 * caller, which is the dispatch shape rather than missing evidence. That table
 * is indexed with the work record's state byte at +1 by the exact same-target
 * dispatcher dispatchStateByte1Table3e00WorldWorld03Area13213_801F307C, whose
 * entries are this overlay's step handlers. The advance-and-clamp role comes
 * from this body's own immediates and from entry 0's seeding of the same
 * fields; the countdown re-arm and the state-byte advance come from the byte
 * cells the body writes.
 *
 * Shape evidence: the world02/area103 overlay holds a copy of these 0xC0 bytes
 * at 0x801F37A0 and the world03/area134 overlay at 0x801F4400, each differing
 * in exactly the two immediate bytes of the jal at body offset 84 (0x801F38DC
 * there against 0x801F3268 here), compared byte for byte against this target's
 * payload. A relocation-different sibling is shape evidence only, never clone
 * eligibility. The statement order, the pointer-cell reload points and the u8
 * accumulator spelling of the countdown group follow the exact
 * advanceWorkWord1CClampedToWord14AndRearmCountdownThenAdvanceWorkByte1World02Area10313_801F37A0;
 * this function's own literals, clamp limit, step, sweep value and call target
 * come from its original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWord1CClampedToWord14AndRearmCountdownThenAdvanceWorkByte1World03Area13213_801F312C(
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
  drawTexturedRibbonAlongWorkSpanWorld03Area13213_801F3268();
  cursor = D_1F800044;
  countdown = (u8)(cursor[9] - 1);
  cursor[9] = countdown;
  if (countdown == 0) {
    D_1F800044[9] = 0x10;
    D_1F800044[1]++;
  }
}
