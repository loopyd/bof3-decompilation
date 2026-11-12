#include "bof3/ui/shop00_internal.h"

/* @source 0x801D3B98
 * @behavior shop UI sub-step handler reached as entry 1 of the sub-step table
 *           D_801E52D8, which the dispatcher func_801D3AA8 selects with the UI
 *           sub-step byte D_80148652: it emits the shop panel strip through
 *           func_801DAB90 with the fixed panel arguments (0x14, 0x12, 0x118,
 *           0x13) and the main-RAM CLUT-bank byte D_80144952 as the fifth
 *           argument. When the saved-state byte D_8014865C is not the -1
 *           sentinel it formats ten times the main-RAM byte D_80148699 into the
 *           shared text buffer D_80145AD4 through sprintf with this EMI's "%d"
 *           format string D_801D0C1C, submits that buffer through
 *           func_8016620C with the constants 0 and 8, submits the texture row
 *           0x1C at row 0x16 through func_8014F800 with the constant 0xFF as
 *           the fourth argument and the value returned by func_801502D0(0xA1)
 *           as the fifth, and draws the money panel through
 *           drawShopMoneyPanel at (0x62, 0x28) with the constant 0 and the
 *           main-RAM value D_80144F50. Finally, when bit 1 of the global gate
 *           halfword D_801490A4 is set, it re-arms the frame timer phaseTimer
 *           with 6, advances the UI sub-step byte D_80148652 by one and plays
 *           the cue 0x102 through the main-exe cue dispatcher func_8015DF18.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitShopPanelStripAndAmountThenAdvanceUiSubstepOnGate(void) {
  /* Plain view of the volatile frame-timer cell: the guarded schedule keeps
   * the re-arm constant in the branch delay slot and hoists the sub-step load
   * above the store, which the volatile view's ordering barriers defeat (the
   * same shape as the sibling func_801E2084). */
  u8* timer = (u8*)&phaseTimer;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  if (D_8014865C != -1) {
    /* The amount is the shared header cell scaled by ten; the sibling sub-step
     * handler func_801D3D84 charges the same product against D_80144F50.
     * The func_801502D0 result is passed in place as the fifth argument of
     * func_8014F800, the shape the exact sibling
     * retreatShopPanelRowThenAdvanceUiSubstep uses for the same group. */
    sprintf((char*)D_80145AD4, D_801D0C1C, D_80148699 * 10);
    func_8016620C(0, 8, (u32)D_80145AD4);
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0xA1));
    drawShopMoneyPanel(0x62, 0x28, 0, D_80144F50);
  }
  if (D_801490A4 & 2) {
    *timer = 6;
    D_80148652 += 1;
    func_8015DF18(0x102);
  }
}
