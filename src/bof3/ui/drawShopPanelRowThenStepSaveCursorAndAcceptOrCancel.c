#include "bof3/ui/shop00_internal.h"

/* @source 0x801D4E74
 * @behavior shop sub-step handler at table entry 8 of the sub-step handler
 *           table D_801E530C, which func_801D46A4 dispatches with the UI
 *           sub-step byte D_80148652 as index: it emits the shop panel strip
 *           through this EMI's panel strip emitter func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the value
 *           returned by func_801502D0(0x9D) as its fifth argument, and draws
 *           the panel row through the row emitter func_801D67B0 at the fixed
 *           coordinates (0x20, 0x30, 1).
 *           It then steps the selected row D_801E6074 of the save-slot list
 *           with the pad state D_80145AA8 masked by 0x5000 through the shared
 *           direction decoder func_8014E640: the 0x1000 bit decrements the row
 *           and wraps it to the last row when the decremented byte is negative,
 *           the 0x4000 bit increments it and wraps it to 0 when the increment
 *           exceeds that last row, and the last row is the published entry
 *           count D_801E6078 minus one, widened by one while the
 *           remaining-capacity byte D_801E60FC is nonzero and D_801E6078 is not
 *           3. When the stored row differs from the row it read, it plays the
 *           cursor cue 0x101 through the cue dispatcher func_8015DF18.
 *           While the pad state shares a bit with the accept mask D_80145AC2
 *           it plays the accept cue 0x104 and reads the slot byte of the
 *           selected panel record, the byte of the record table base
 *           D_801E60AD at 0x1C times the stored row: an empty (0xFF) slot with
 *           remaining capacity clears the work-block byte D_8014865B and arms
 *           the sub-step byte with 0xD, an empty slot without capacity plays
 *           the cue 0x107 and arms the sub-step byte with 0xC together with the
 *           panel action id D_8018E228 held at 4, and an occupied slot reads
 *           the sub-step byte into a local, clears D_8014865B and arms the
 *           sub-step byte with that value incremented by one. While it instead
 *           shares a bit with the cancel mask D_80145AC4 it plays the cancel
 *           cue 0x106 and arms the sub-step byte with 0xB.
 * @status partial
 * @match 89.80
 * @residual Increment-arm register allocation and store scheduling differ at
 *           the original 588-byte size. Reusing the consumed direction result
 *           for the byte-truncated row restores the early truncation, but the
 *           cursor store still follows the capacity/count loads and stores the
 *           truncated value rather than the original's untruncated sum.
 */
void drawShopPanelRowThenStepSaveCursorAndAcceptOrCancel(void) {
  s32 direction;
  u8 cursor;
  u8 extra;
  u8 last;
  s32 widen;
  u8 next;
  u8 substep;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x9D));
  func_801D67B0(0x20, 0x30, 1);
  direction = func_8014E640(D_80145AA8 & 0x5000);
  cursor = D_801E6074;
  if (direction & 0x1000) {
    next = cursor - 1;
    D_801E6074 = next;
    /* The original keeps the decremented row in the register and tests the
     * sign of its low byte, so the wrap test is the byte view of that value. */
    if ((s8)next < 0) {
      extra = 0;
      if (D_801E60FC != 0) {
        extra = D_801E6078 != 3;
      }
      D_801E6074 = D_801E6078 + extra - 1;
    }
  } else if (direction & 0x4000) {
    direction = (u8)(cursor + 1);
    D_801E6074 = direction;
    /* The original reads the entry count once for both the capacity widening
     * and the bound test of this arm, so it stays a local here. */
    last = D_801E6078;
    widen = 0;
    if (D_801E60FC != 0) {
      widen = last != 3;
    }
    if (last + widen - 1 < direction) {
      D_801E6074 = 0;
    }
  }
  if (cursor != D_801E6074) {
    func_8015DF18(0x101);
  }
  if (D_80145AA8 & D_80145AC2) {
    func_8015DF18(0x104);
    if ((&D_801E60AD)[D_801E6074 * 0x1C] == 0xFF) {
      if (D_801E60FC != 0) {
        D_8014865B = 0;
        D_80148652 = 0xD;
      } else {
        func_8015DF18(0x107);
        D_80148652 = 0xC;
        D_8018E228 = 4;
      }
    } else {
      /* The sub-step read precedes the arming stores, so it stays a local
       * placed at the original's load position. */
      substep = D_80148652;
      D_8014865B = 0;
      D_80148652 = substep + 1;
    }
  } else if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x106);
    D_80148652 = 0xB;
  }
}
