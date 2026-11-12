#include "bof3/ui/shop00_internal.h"

/* @source 0x801D5BB0
 * @behavior shop sub-step handler at table entry 0x10 of the sub-step handler
 *           table D_801E530C, which func_801D46A4 dispatches with the UI
 *           sub-step byte D_80148652 as index: it emits the shop panel strip
 *           through this EMI's panel strip emitter func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the value
 *           returned by func_801502D0(0x92) as its fifth argument, and draws
 *           the sliding shop value panel through func_801DDFB0 at the origin
 *           (0x62, 0x4C) with the overlay flag byte D_801E60F8, the main-RAM
 *           byte D_8018E260 and the variant constant 1.
 *           When the two-option prompt helper drawTwoOptionPromptThenReportChoice reports a choice
 *           through a nonzero low byte, the step either accepts or cancels.
 *           While the work-block byte D_8014865B is nonzero it plays the accept
 *           cue 0x104 through the cue dispatcher func_8015DF18, reads the UI
 *           sub-step byte into a local, arms the overlay-local transition
 *           variant byte D_801E60EC with 0x15, sets the sub-step byte to 0x13
 *           (the panel transition handler func_801D5E88, which switches on that
 *           variant byte), re-arms the frame timer phaseTimer with 2 and
 *           archives the value it read incremented by one into D_801E6094,
 *           which that transition handler restores into the sub-step byte once
 *           phaseTimer expires; while that byte is zero it plays the cancel cue
 *           0x106 and arms the sub-step byte with 4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopValuePanelThenArmAcceptTransition(void) {
  u8 previous;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x92));
  func_801DDFB0(0x62, 0x4C, D_801E60F8, D_8018E260, 1);
  if ((u8)drawTwoOptionPromptThenReportChoice()) {
    /* The original tests the work-block byte with a signed load (lb) on the
     * bare symbol expression rather than through a pointer local, because the
     * accept arm stores three other cells before it. */
    if (*(s8*)&D_8014865B) {
      func_8015DF18(0x104);
      /* The sub-step read precedes the three arming stores and its
       * increment-then-store pair lands on D_801E6094 after them, so the read
       * stays a local placed at the original's load position. */
      previous = D_80148652;
      D_801E60EC = 0x15;
      D_80148652 = 0x13;
      phaseTimer = 2;
      D_801E6094 = previous + 1;
    } else {
      func_8015DF18(0x106);
      D_80148652 = 4;
    }
  }
}
