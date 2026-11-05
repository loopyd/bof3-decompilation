#include "bof3/ui/shop00_internal.h"

/* @source 0x801D446C
 * @behavior shop UI sub-step handler at entry 4 of the sub-step table
 *           D_801E52F0, which the dispatcher func_801D41B0 selects with the UI
 *           sub-step byte D_80148652: emits the shop panel strip through
 *           func_801DAB90 with the fixed panel arguments (0x14, 0x12, 0x118,
 *           0x13) and the main-RAM CLUT-bank byte D_80144952 as the fifth
 *           argument. When the global gate halfword D_80143C40 is set the
 *           handler returns after the strip. When the gate is clear it calls
 *           this target's record rebuild helper func_801D68D0 with the
 *           constant 1, arms the work-block byte D_8014865B with 1, advances
 *           the UI sub-step byte D_80148652 by one, clears the two main-RAM
 *           halfword cells D_80145AC8/D_80145AC6 and, unless the saved-state
 *           byte D_8014865C still holds the -1 sentinel, plays the main-exe cue
 *           0xD5 through the cue dispatcher func_80150284. It then walks the
 *           four signed halfwords D_801E6100[0..3] and calls func_8016BB6C
 *           with the constant 0 and the entry index for every entry that is
 *           not zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitPanelStripThenRebuildShopStateOnClearGate(void) {
  s16 i;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  if (D_80143C40 == 0) {
    func_801D68D0(1);
    D_8014865B = 1;
    D_80148652 = D_80148652 + 1;
    D_80145AC8 = 0;
    D_80145AC6 = 0;
    if (D_8014865C != -1) {
      func_80150284(0xD5);
    }
    for (i = 0; i < 4; i++) {
      if (D_801E6100[i] != 0) {
        func_8016BB6C(0, i);
      }
    }
  }
}
