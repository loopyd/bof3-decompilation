#include "bof3/bof3.h"

extern u8 *D_1F800044;
void func_801F88FC(u8 *work);

/* @source 0x801F853C
 * @behavior Calls drawing helper 0x801F88FC with work plus 0x0C, subtracts
 * 16 from halfword 0x12 with signed zero clamping, and subtracts 16 from
 * halfword 0x14 when signed halfword 0x12 is below 128. If signed halfword
 * 0x14 is negative, reloads the scratchpad work pointer, clears the original
 * work halfword 0x12, and increments byte 1 through the reloaded pointer.
 * Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawAndShrinkWorkHalfwordsThenAdvanceScenarioScena0300_801F853C(void) {
  u8 *work;
  u8 *next_work;

  work = D_1F800044;
  func_801F88FC(work + 0x0C);
  FIELD_REF(s16, work, 0x12) -= 16;
  if (FIELD_REF(s16, work, 0x12) < 0) {
    FIELD_REF(s16, work, 0x12) = 0;
  }
  if (FIELD_REF(s16, work, 0x12) < 128) {
    FIELD_REF(s16, work, 0x14) -= 16;
  }
  if (FIELD_REF(s16, work, 0x14) < 0) {
    next_work = D_1F800044;
    FIELD_REF(s16, work, 0x12) = 0;
    next_work[1]++;
  }
}
