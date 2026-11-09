#include "bof3/bof3.h"

extern u8 *D_1F800044;
void buildFourRotatedQuadCornerVerticesFromWorkCentreAndAnglesScenarioScena0300_801F7FFC(
    u8 *work);
void drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170(
    u8 *work);

/* @source 0x801F7B10
 * @behavior Calls local helpers 0x801F7FFC and 0x801F8170 with the captured
 * work pointer plus 0x0C, then increments that work object's halfword at
 * 0x5E. Decrements unsigned byte 9 of the current scratchpad work object;
 * at zero reloads it with 8 and increments work byte 1. Captures the
 * countdown pointer before the halfword store and reloads after each byte
 * store that can alias its cell. This is entry 1 of the byte-1 handler
 * table at 0x801FCF70. Takes no arguments and returns no value. The
 * halfword's wider role is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownReload8IncrementWorkHalfword5EAndAdvanceWorkByte1ScenarioScena0300_801F7B10(void) {
  u8 *work;
  u8 *helper_work;
  u8 *countdown_work;
  u32 halfword;

  work = D_1F800044;
  helper_work = work + 0x0C;
  buildFourRotatedQuadCornerVerticesFromWorkCentreAndAnglesScenarioScena0300_801F7FFC(
      helper_work);
  drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170(
      helper_work);
  halfword = *(u16 *)(work + 0x5E);
  countdown_work = D_1F800044;
  *(u16 *)(work + 0x5E) = halfword + 1;
  if (--countdown_work[9] == 0) {
    D_1F800044[9] = 8;
    D_1F800044[1]++;
  }
}
