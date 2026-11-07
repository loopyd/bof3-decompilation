#include "bof3/ui/shop00_internal.h"

/* @source 0x801D41EC
 * @behavior shop UI sub-step handler at entry 0 of the sub-step table
 *           D_801E52F0, which the dispatcher func_801D41B0 selects with the UI
 *           sub-step byte D_80148652: emits the shop panel strip through
 *           func_801DAB90 with the fixed panel arguments (0x14, 0x12, 0x118,
 *           0x13) and the main-RAM CLUT-bank byte D_80144952 as the fifth
 *           argument. When the saved-state byte D_8014865C is not the -1
 *           sentinel it submits the texture row 0x1C at row 0x16 through
 *           func_8014F800 with the constant 0xFF as the fourth argument and
 *           the value returned by func_801502D0(0xA4) as the fifth. It then
 *           counts the shared frame timer phaseTimer down and returns while
 *           the decremented byte is not zero; once it wraps it advances the UI
 *           sub-step byte D_80148652 by one, switches the frontend-local mode
 *           through func_8014ECAC with the bare constant 0, and for the four
 *           row indices 0 through 3 publishes the main-exe per-row flag
 *           func_8016B344(0, row) into the overlay halfword table D_801E6100
 *           and, for every nonzero flag, sets that row up through
 *           func_8016B968(0, row).
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopPanelStripThenPublishRowFlagsOnTimerExpiry(void) {
  u32 handle;
  u8  timer;
  s16 i;
  s16 flag;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  if (D_8014865C != -1) {
    handle = func_801502D0(0xA4);
    func_8014F800(0x1C, 0x16, 0, 0xFF, handle);
  }
  timer = phaseTimer - 1;
  phaseTimer = timer;
  if (timer == 0) {
    D_80148652 = D_80148652 + 1;
    func_8014ECAC(0);
    for (i = 0; i < 4; i++) {
      flag = func_8016B344(0, i);
      D_801E6100[i] = flag;
      if (flag != 0) {
        func_8016B968(0, i);
      }
    }
  }
}
