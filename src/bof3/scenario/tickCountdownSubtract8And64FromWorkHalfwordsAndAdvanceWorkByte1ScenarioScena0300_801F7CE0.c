#include "bof3/bof3.h"

extern u8 *D_1F800044;
void func_801F7FFC(u8 *work);
void drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170(
    u8 *work);

/* @source 0x801F7CE0
 * @behavior Calls local helpers 0x801F7FFC and 0x801F8170 with the captured
 * work pointer plus 0x0C, then subtracts 8 from captured work halfword 0x5A
 * and 64 from halfword 0x5C. Decrements unsigned byte 9 of the current
 * scratchpad work object and increments work byte 1 when it reaches zero.
 * Captures the countdown pointer before the second halfword store and
 * reloads it after the byte store, preserving possible cell aliasing.
 * Takes no arguments and returns no value. The halfwords' roles are unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownSubtract8And64FromWorkHalfwordsAndAdvanceWorkByte1ScenarioScena0300_801F7CE0(void) {
  u8 *work;
  u8 *helper_work;
  u8 *countdown_work;
  u32 halfword;

  work = D_1F800044;
  helper_work = work + 0x0C;
  func_801F7FFC(helper_work);
  drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170(
      helper_work);
  *(u16 *)(work + 0x5A) -= 8;
  halfword = *(u16 *)(work + 0x5C);
  countdown_work = D_1F800044;
  *(u16 *)(work + 0x5C) = halfword - 64;
  if (--countdown_work[9] == 0) {
    D_1F800044[1]++;
  }
}
