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

/* @source 0x801F507C
 * @behavior Overlay step handler, entry index 2 of the four-pointer work-byte-2
 *           handler table at 0x801F5A28. It reads the work record published
 *           through the scratchpad pointer cell 0x1F800044 and hands the drawer
 *           drawEightShadedTexturedQuadsSteppingDepthAtWorkPositionWorld02Area10313_801F5174
 *           a ramp pair derived from that record's byte at 0x09:
 *           (0x1F - byte) / 4 as the per-band step and (0x1F - byte) * 4 as the
 *           shade level, which draws eight shaded textured quads whose step and
 *           shade level both shrink as the byte grows. It then re-reads the cell
 *           and increments byte 0x09, and when a fresh cell read shows that byte
 *           has reached 0x19 it clears byte 0x09 and advances the record's
 *           work-byte-2 dispatch step. Takes no arguments, returns nothing and
 *           uses a 0x18-byte frame for the saved return address.
 * @status exact
 * @match 100.00
 * @residual none
 * The increment group needs its own pointer variable: the original keeps that
 * reload in $v0 and the two later groups in $v1, while one variable shared
 * across all three takes $v1 for the whole run and matches 32 of 40
 * instructions. Reusing the drawing group's variable for it measures the same
 * 32, so the split is the narrowest exact form.
 */
void drawWorkRampStepAndAdvanceWorkByte2At19World02Area10313_801F507C(void) {
  u8* drawCell;
  u8* incCell;
  u8* stepCell;
  s32 ramp;

  drawCell = D_1F800044;
  ramp = 0x1F - drawCell[9];
  drawEightShadedTexturedQuadsSteppingDepthAtWorkPositionWorld02Area10313_801F5174(
      ramp / 4, ramp * 4);
  incCell = D_1F800044;
  incCell[9]++;
  stepCell = D_1F800044;
  if (stepCell[9] >= 0x19) {
    stepCell[9] = 0;
    stepCell = D_1F800044;
    stepCell[2]++;
  }
}
