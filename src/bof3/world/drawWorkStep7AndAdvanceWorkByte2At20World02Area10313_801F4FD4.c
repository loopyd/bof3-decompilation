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
void func_801F53F0(s32 arg0);

/* @source 0x801F4FD4
 * @behavior Overlay step handler, entry 1 of the four-pointer work-byte-2
 *           handler table at 0x801F5A28. It hands the band drawer
 *           drawEightShadedTexturedQuadsSteppingDepthAtWorkPositionWorld02Area10313_801F5174
 *           the fixed pair (7, 0x80) - the hardest per-band step of the 0x1F
 *           down ramp together with the 0x80 shade level - then reads the work
 *           record published through the scratchpad pointer cell 0x1F800044 and,
 *           only while that record's byte 0x09 is below 0x10, hands the byte to
 *           the arg-scaled quad drawer func_801F53F0. It then increments byte
 *           0x09 through a fresh cell read and, when one more cell read shows
 *           the byte has reached 0x20, clears byte 0x09 and advances the
 *           record's work-byte-2 dispatch step. Takes no arguments, returns
 *           nothing and uses a 0x18-byte frame for the saved return address.
 * @status exact
 * @match 100.00
 * @residual none
 * Two pointer locals behind the drawing group: the original keeps the increment
 * group's reload in $v0 and the step group's in $v1, as its two exact siblings in
 * this overlay do. The guarded call needs its own byte local: the original holds
 * the loaded byte in $a0, tests it with sltiu and passes that same register to
 * func_801F53F0, where re-reading the cell in the call argument would reload it.
 */
void drawWorkStep7AndAdvanceWorkByte2At20World02Area10313_801F4FD4(void) {
  u8* drawCell;
  u8* incCell;
  u8* stepCell;
  u8 step;

  drawEightShadedTexturedQuadsSteppingDepthAtWorkPositionWorld02Area10313_801F5174(
      7, 0x80);
  drawCell = D_1F800044;
  step = drawCell[9];
  if (step < 0x10) {
    func_801F53F0(step);
  }
  incCell = D_1F800044;
  incCell[9]++;
  stepCell = D_1F800044;
  if (stepCell[9] >= 0x20) {
    stepCell[9] = 0;
    stepCell = D_1F800044;
    stepCell[2]++;
  }
}
