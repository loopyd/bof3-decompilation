#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

void drawEightShadedTexturedQuadsSteppingDepthAtWorkPositionWorld02Area10313_801F5174(
    s32 arg0, s32 arg1);
void drawRadialRingOfSixtyFourTintedCellsScaledByArgumentAroundProjectedWorkPositionWorld02Area10313_801F53F0(
    s32 arg0);

/* @source 0x801F4F44
 * @behavior Overlay step handler, entry 0 of the work-byte-2 handler table at
 *           0x801F5A28. It re-reads the work-record pointer cell at scratchpad
 *           0x1F800044 for each access group: hands the record's byte at 0x09
 *           to the drawer
 *           drawEightShadedTexturedQuadsSteppingDepthAtWorkPositionWorld02Area10313_801F5174
 *           together with 0x80, hands that same byte to the drawer
 *           drawRadialRingOfSixtyFourTintedCellsScaledByArgumentAroundProjectedWorkPositionWorld02Area10313_801F53F0,
 *           increments byte 0x09 through a
 *           fresh cell read, then re-reads the cell once more and advances the
 *           byte-2 dispatch step when byte 0x09 has reached 8. Both callees
 *           emit display-list primitives (SetPolyFT4 with RotTransPers4, and
 *           SetDrawMode with RotTransPers), so byte 0x09 is a drawing step
 *           counter. Takes no arguments, returns nothing and uses a 0x18-byte
 *           frame for the saved return address.
 * @status exact
 * @match 100.00
 * @residual none
 * Two pointer locals, not one: the original keeps the two drawing groups in
 * $v0 and the two step groups in $v1, and one variable reused across all four
 * groups becomes a single pseudo that takes $v1 throughout.
 */
void drawWorkStepAndAdvanceWorkByte2At8World02Area10313_801F4F44(void) {
  u8* drawCell;
  u8* stepCell;

  drawCell = D_1F800044;
  drawEightShadedTexturedQuadsSteppingDepthAtWorkPositionWorld02Area10313_801F5174(
      drawCell[9], 0x80);
  drawCell = D_1F800044;
  drawRadialRingOfSixtyFourTintedCellsScaledByArgumentAroundProjectedWorkPositionWorld02Area10313_801F53F0(
      drawCell[9]);
  stepCell = D_1F800044;
  stepCell[9]++;
  stepCell = D_1F800044;
  if (stepCell[9] >= 8) {
    stepCell[2]++;
  }
}
