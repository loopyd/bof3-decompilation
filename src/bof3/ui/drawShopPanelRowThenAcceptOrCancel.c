#include "bof3/ui/shop00_internal.h"

/* @source 0x801D50C0
 * @behavior shop sub-step handler at table entry 9 of the sub-step handler
 *           table D_801E530C, which func_801D46A4 dispatches with the UI
 *           sub-step byte D_80148652 as index: it emits the shop panel strip
 *           through this EMI's panel strip emitter func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the value
 *           returned by func_801502D0(0x9F) as its fifth argument, and draws
 *           the panel row through the row emitter func_801D67B0 at the fixed
 *           coordinates (0x20, 0x30, 0).
 *           When the two-option prompt helper func_801DA678 reports a choice
 *           through a nonzero low byte, the step either accepts or cancels.
 *           While the work-block byte D_8014865B is nonzero it plays the accept
 *           cue 0x104 through the cue dispatcher func_8015DF18, reads the UI
 *           sub-step byte into a local, arms the overlay-local byte D_801E60EC
 *           with 0x14 (the variant byte the sub-step 0x13 handler func_801D5E88
 *           switches on), sets the sub-step byte to 0x13, re-arms the frame
 *           timer phaseTimer with 2 and archives the value it read incremented
 *           by one into D_801E6094; while that byte is zero it plays the cancel
 *           cue 0x106 and steps the sub-step byte back by one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopPanelRowThenAcceptOrCancel(void) {
  u8 previous;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x9F));
  func_801D67B0(0x20, 0x30, 0);
  if ((u8)func_801DA678()) {
    /* The original tests the work-block byte with a signed load (lb) on the
     * bare symbol expression rather than through a pointer local, because the
     * accept arm stores three other cells before it. */
    if (*(s8*)&D_8014865B) {
      func_8015DF18(0x104);
      /* The sub-step read precedes the three arming stores and its
       * increment-then-store pair lands on D_801E6094 after them, so the read
       * stays a local placed at the original's load position. */
      previous = D_80148652;
      D_801E60EC = 0x14;
      D_80148652 = 0x13;
      phaseTimer = 2;
      D_801E6094 = previous + 1;
    } else {
      func_8015DF18(0x106);
      D_80148652 = D_80148652 - 1;
    }
  }
}
