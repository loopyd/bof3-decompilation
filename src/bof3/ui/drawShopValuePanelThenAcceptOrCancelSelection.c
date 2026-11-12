#include "bof3/ui/shop00_internal.h"

/* @source 0x801D5AD0
 * @behavior shop sub-step handler at table entry 0x0F of the sub-step handler
 *           table D_801E530C (dispatched through func_801D46A4 with the UI
 *           sub-step byte D_80148652 as index): emits the shop panel strip
 *           through this EMI's panel strip emitter func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the value
 *           returned by func_801502D0(0x91) as its fifth argument, then draws
 *           the sliding shop value panel through func_801DDFB0 at the origin
 *           (0x62, 0x4C) with the overlay flag byte D_801E60F8, the main-RAM
 *           byte D_8018E260 and the variant constant 1.
 *           When the two-option prompt helper drawTwoOptionPromptThenReportChoice reports a choice
 *           through a nonzero low byte, the step either accepts the armed
 *           selection or cancels: while the work-block byte D_8014865B is
 *           nonzero it plays the accept cue 0x104 through the cue dispatcher
 *           func_8015DF18, clears that work-block byte and advances the UI
 *           sub-step byte D_80148652 by one; while that byte is zero it plays
 *           the cancel cue 0x106 and arms the sub-step byte with 4 instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopValuePanelThenAcceptOrCancelSelection(void) {
  s8* armed;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x91));
  func_801DDFB0(0x62, 0x4C, D_801E60F8, D_8018E260, 1);
  if ((u8)drawTwoOptionPromptThenReportChoice()) {
    /* The original materializes this cell's address once and reuses that base
     * for the signed load and the clear store (lui+addiu / lb 0() / sb 0()
     * through one callee-saved register), so both accesses go through the
     * pointer. */
    armed = (s8*)&D_8014865B;
    if (*armed) {
      func_8015DF18(0x104);
      *armed = 0;
      D_80148652 = D_80148652 + 1;
    } else {
      func_8015DF18(0x106);
      D_80148652 = 4;
    }
  }
}
