#include "bof3/ui/shop00_internal.h"

/* @source 0x801D456C
 * @behavior shop UI sub-step handler at entry 5 of the sub-step table
 *           D_801E52F0, which the dispatcher func_801D41B0 selects with the UI
 *           sub-step byte D_80148652: emits the shop panel strip through
 *           func_801DAB90 with the fixed panel arguments (0x14, 0x12, 0x118,
 *           0x13) and the main-RAM CLUT-bank byte D_80144952 as the fifth
 *           argument. When the saved-state byte D_8014865C holds the -1
 *           sentinel it submits the texture row 0x1C at row 0x16 through
 *           func_8014F800 with the value returned by func_801502D0(0x1D) and
 *           takes the advance decision from the low byte of func_801DA678;
 *           otherwise the decision is bit 2 of the global gate halfword
 *           D_801490A4. A nonzero decision advances the UI sub-step byte
 *           D_80148652 by one.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The gate arm is written first, so the sentinel arm is the one the original
 * places out of line behind a forward jump. The func_801502D0 result goes
 * through a local because the original stores it into the outgoing argument
 * slot directly after that call, before the func_8014F800 argument setup;
 * passing it in place, as the sibling appendDimTileOrRedrawPanelStrip does,
 * puts the store in the later `jal` delay slot instead.
 */
void redrawPanelStripThenConditionallyAdvanceUiSubstep(void) {
  u32 prim;
  u32 advance;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  if (D_8014865C != -1) {
    advance = D_801490A4 & 0x2;
  } else {
    prim = func_801502D0(0x1D);
    func_8014F800(0x1C, 0x16, 0, 0xFF, prim);
    advance = func_801DA678() & 0xFF;
  }
  if (advance != 0) {
    D_80148652 += 1;
  }
}
