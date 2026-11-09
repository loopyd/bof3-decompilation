#include "bof3/bof3.h"

extern u8 *D_1F800044;
void func_801F7FFC(u8 *work);
void drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170(
    u8 *work);

/* @source 0x801F7C50
 * @behavior Calls local helpers 0x801F7FFC and 0x801F8170 with the same
 * work pointer plus 0x0C, then decrements unsigned work byte 9. When the
 * decremented byte is zero, reloads it with 128 and increments work byte 1.
 * Reloads the scratchpad work pointer after each store that can alias its
 * cell. Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownReload128AndAdvanceWorkByte1ScenarioScena0300_801F7C50(void) {
  u8 *work;

  work = D_1F800044 + 0x0C;
  func_801F7FFC(work);
  drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170(
      work);
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 128;
    D_1F800044[1]++;
  }
}
