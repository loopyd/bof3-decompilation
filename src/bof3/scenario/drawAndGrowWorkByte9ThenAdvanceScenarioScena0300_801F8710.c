#include "bof3/bof3.h"

extern u8 *D_1F800044;
void func_801F8C04(s32 x, s32 y, s16 radius, u8 center, u8 edge);

/* @source 0x801F8710
 * @behavior Calls local drawing helper 0x801F8C04 with signed work
 * halfwords at 0x2E and 0x30, unsigned byte 9, 0x80 and 0. Adds 15 to
 * work byte 9 with byte wrapping, then reloads the scratchpad work pointer.
 * If the reloaded byte exceeds 60, sets it to 255 and increments work
 * byte 1 through another pointer reload. Takes no arguments and returns
 * no value. Pointer reloads preserve stores that can alias the pointer cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawAndGrowWorkByte9ThenAdvanceScenarioScena0300_801F8710(void) {
  u8 *draw_work;
  u8 *test_work;

  draw_work = D_1F800044;
  func_801F8C04(FIELD_REF(s16, draw_work, 0x2E),
                FIELD_REF(s16, draw_work, 0x30), draw_work[9], 0x80, 0);
  D_1F800044[9] += 15;
  test_work = D_1F800044;
  if (test_work[9] > 60) {
    test_work[9] = 255;
    D_1F800044[1]++;
  }
}
