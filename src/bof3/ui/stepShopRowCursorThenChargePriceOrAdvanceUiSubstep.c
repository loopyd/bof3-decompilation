#include "bof3/ui/shop00_internal.h"

/* @source 0x801D3D84
 * @behavior shop UI sub-step handler at entry 3 of the sub-step table
 *           D_801E52D8, which the dispatcher func_801D3AA8 selects with the UI
 *           sub-step byte D_80148652: draws the three-row selector panel
 *           through drawThreeRowSelectorPanel at the fixed origin (0x6E, 0x4C),
 *           emits the shop panel strip through func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument and, when the
 *           saved-state byte D_8014865C is not the -1 sentinel, submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the
 *           constant 0xFF as the fourth argument and the value returned by
 *           func_801502D0(0xA1) as the fifth, then draws the money panel
 *           through drawShopMoneyPanel at (0x62, 0x28) with the constant 0 and
 *           the main-RAM value D_80144F50. It then walks the overlay row index
 *           D_801E60F0 with the pad state D_80145AA8: bit 0x1000 steps the row
 *           down and wraps the byte back to 2 when the decremented value turns
 *           negative, bit 0x4000 steps it up and wraps it back to 0 once it
 *           exceeds 2, and a row that changed plays cue 0x101 through
 *           func_8015DF18. On the pressed mask D_80145AC2 it reads the row,
 *           clears the frame timer phaseTimer and the overlay flag D_801E60F4
 *           and then either plays cue 0x104 and advances the sub-step byte by
 *           two when the row is not 0, or charges the amount ten times the
 *           byte D_80148699: when the main-RAM value D_80144F50 is below that
 *           product it plays cue 0x107 and advances the sub-step by one,
 *           otherwise it plays cue 0x104, subtracts the product from D_80144F50
 *           and advances by two. On the cancel mask D_80145AC4 it plays cue
 *           0x106, resets phaseTimer, sets D_801E60F4 to 1 and advances the
 *           sub-step by two.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepShopRowCursorThenChargePriceOrAdvanceUiSubstep(void) {
  u16 pad;
  u8 row;

  drawThreeRowSelectorPanel(0x6E, 0x4C);
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  if (D_8014865C != -1) {
    /* The texture handle is passed in place as the fifth argument of
     * func_8014F800 instead of through a local: the original keeps it in $v0
     * and stores it into the outgoing argument slot in the call's delay slot
     * (sibling shape: retreatShopPanelRowThenAdvanceUiSubstep). */
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0xA1));
    drawShopMoneyPanel(0x62, 0x28, 0, D_80144F50);
  }
  /* The pad state and the current row are read once before the two cursor
   * arms; the original keeps both in registers across them. */
  pad = D_80145AA8;
  row = D_801E60F0;
  if (pad & 0x1000) {
    /* The decrement test is the signed byte test (sll 24 / bgez), so the
     * wrapped 0xFF is caught while the increment test below is the unsigned
     * byte bound (andi 0xFF / sltiu 3). */
    D_801E60F0 = row - 1;
    if ((s8)D_801E60F0 < 0) {
      D_801E60F0 = 2;
    }
  } else if (pad & 0x4000) {
    D_801E60F0 = row + 1;
    if (D_801E60F0 > 2) {
      D_801E60F0 = 0;
    }
  }
  if (row != D_801E60F0) {
    func_8015DF18(0x101);
  }
  /* The pressed-state halfword is re-read here rather than reusing the pair
   * above: the original reloads it (lhu) before masking it with the two
   * pressed-button masks, and the fresh read is what keeps the second test
   * independent of the cursor arms. */
  if (D_80145AA8 & D_80145AC2) {
    u8 cursor;

    cursor = D_801E60F0;
    phaseTimer = 0;
    D_801E60F4 = 0;
    /* The original lays the row-0 arm out as the fall-through block and keeps
     * the other arm and the insufficient-funds arm out of line, so the source
     * tests the row for zero and writes the paid path first. */
    if (cursor == 0) {
      /* Both cells are taken through pointer locals because the original
       * materializes their addresses once into callee-saved registers and
       * reloads them after the cue call (s0/s1); the folded symbol form
       * would re-materialize each access instead. The money comparison is the
       * unsigned sltu, so both values are taken at unsigned width. */
      u8*  units;
      u32* amount;

      units = &D_80148699;
      amount = (u32*)&D_80144F50;
      if (*amount >= (u32)*units * 10u) {
        func_8015DF18(0x104);
        *amount = *amount - (u32)*units * 10u;
        D_80148652 = D_80148652 + 2;
      } else {
        func_8015DF18(0x107);
        D_80148652 = D_80148652 + 1;
      }
    } else {
      func_8015DF18(0x104);
      D_80148652 = D_80148652 + 2;
    }
  } else if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x106);
    phaseTimer = 0;
    D_801E60F4 = 1;
    D_80148652 = D_80148652 + 2;
  }
}
