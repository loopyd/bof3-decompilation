#include "bof3/ui/shop00_internal.h"

/* @source 0x801D3FD4
 * @behavior shop UI sub-step handler at entry 4 of the sub-step table
 *           D_801E52D8, which the dispatcher func_801D3AA8 selects with the UI
 *           sub-step byte D_80148652: draws the three-row selector panel through
 *           drawThreeRowSelectorPanel at the fixed origin (0x6E, 0x4C), emits
 *           the shop panel strip through func_801DAB90 with the fixed panel
 *           arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM CLUT-bank byte
 *           D_80144952 as the fifth argument, draws the message panel through
 *           func_801DA2F4 at (0x62, 0x28) with the constant 0 and the main-RAM
 *           value D_80144F50 when the saved-state byte D_8014865C is not the
 *           -1 sentinel, then submits the texture row 0x1C at row 0x16 through
 *           func_8014F800 with the constant 0xFF as the fourth argument and
 *           the value returned by func_801502D0(0xA2) as the fifth. When the
 *           shared pad-state halfword D_80145AA8 is nonzero it finally steps
 *           the UI sub-step byte D_80148652 back by one; a clear halfword
 *           leaves that byte untouched.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The sub-step byte is decremented through a bare symbol expression
 * (`D_80148652 = D_80148652 - 1`), not through a named pointer: the original
 * reads the byte with `lui` plus `lbu %lo(D_80148652)` and writes it back with
 * a second `lui $at` plus `sb %lo`. That is the folded form the exact sibling
 * func_801D3AE4 emits for the same cell (`D_80148652 = D_80148652 + 1`), while
 * the pointer form of advanceUiSubstep and appendDimTileOrRedrawPanelStrip
 * keeps one materialized address.
 *
 * The func_801502D0 result is passed in place as the fifth argument of
 * func_8014F800 instead of through a local: the original keeps it in $v0 and
 * stores it into the outgoing argument slot in the `func_8014F800` delay slot,
 * the same shape as appendDimTileOrRedrawPanelStrip.
 */
void drawShopPanelsThenDecrementUiSubstep(void) {
  drawThreeRowSelectorPanel(0x6E, 0x4C);
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  if (D_8014865C != -1) {
    func_801DA2F4(0x62, 0x28, 0, D_80144F50);
  }
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0xA2));
  if (D_80145AA8 != 0) {
    D_80148652 = D_80148652 - 1;
  }
}
