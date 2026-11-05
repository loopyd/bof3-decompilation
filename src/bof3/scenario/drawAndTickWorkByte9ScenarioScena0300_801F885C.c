#include "bof3/bof3.h"

extern u8 *D_1F800044;
void func_801F8C04(s32 x, s32 y, s16 radius, u8 center, u8 edge);

/* @source 0x801F885C
 * @behavior Calls the local drawing helper at 0x801F8C04 with signed
 * halfwords at work offsets 0x2E and 0x30, unsigned byte 9, 0x80 and 0.
 * Decrements byte 9, reloads the scratchpad work pointer, and advances
 * work byte 1 if the reloaded signed byte 9 is negative. The byte store
 * can alias the scratchpad pointer cell, so its reload is significant.
 * Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawAndTickWorkByte9ScenarioScena0300_801F885C(void) {
  u8 *draw_work;
  u8 *test_work;

  draw_work = D_1F800044;
  func_801F8C04(FIELD_REF(s16, draw_work, 0x2E),
                FIELD_REF(s16, draw_work, 0x30), draw_work[9], 0x80, 0);
  D_1F800044[9]--;
  test_work = D_1F800044;
  if ((s8)test_work[9] < 0) {
    test_work[1]++;
  }
}
