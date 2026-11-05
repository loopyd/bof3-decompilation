#include "bof3/ui/shop00_internal.h"

/* @source 0x801D430C
 * @behavior shop UI sub-step handler at entry 1 of the sub-step table
 *           D_801E52F0, which the dispatcher func_801D41B0 selects with the UI
 *           sub-step byte D_80148652: when the global gate halfword D_80143C40
 *           is clear it appends the fullscreen dim tile through
 *           appendFullscreenDimTile and then advances the UI sub-step byte by
 *           one. When the gate is set and the saved-state byte D_8014865C is
 *           not the -1 sentinel, it re-emits the shop panel strip through
 *           func_801DAB90 with the fixed panel arguments (0x14, 0x12, 0x118,
 *           0x13) and the main-RAM CLUT-bank byte D_80144952 as the fifth
 *           argument, then submits the texture row 0x1C at row 0x16 through
 *           func_8014F800 with the value returned by func_801502D0(0xA4).
 *           When the saved-state byte is the -1 sentinel both cells stay
 *           untouched.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The sub-step byte is advanced through a named pointer: the original
 * materializes the byte's address into one register (lui/addiu) and reads and
 * writes it at offset 0, which is the form a pointer local produces
 * (advanceUiSubstep, func_801D46E0: same cell, same instruction run). A bare
 * `D_80148652 += 1` folds to `lui`/`lbu %lo` plus a second `lui`/`sb %lo`
 * instead (func_801D43A4).
 *
 * The texture row call passes the func_801502D0 result as its fifth argument
 * in place, not through a local: the original keeps that value in $v0 and
 * stores it into the outgoing argument slot in the delay slot of the
 * func_8014F800 `jal`, while the sibling func_801D41EC, which reads the same
 * chain into a local, stores it directly after the first call.
 */
void appendDimTileOrRedrawPanelStrip(void) {
  u8* subStep;

  if (D_80143C40 == 0) {
    appendFullscreenDimTile();
    subStep = &D_80148652;
    *subStep += 1;
  } else if (D_8014865C != -1) {
    func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0xA4));
  }
}
