#include "bof3/bof3.h"

extern u8* scenarioScena0100Work; /* @source 0x1F800044 @kind unknown */
void func_801F7678(u8 value);
s32 updateWorkTableAndReportActiveScenarioScena0100_801F7C54(void);

/* @source 0x801F75C8
 * @behavior Entry 3 of the work-byte-1 handler table at 0x801FE26C.
 * Calls func_801F7678 with unsigned work byte 9, then updateWorkTableAndReportActiveScenarioScena0100_801F7C54,
 * discarding its result. Subtracts four from work byte 9 modulo 256 and
 * increments the state byte at offset 1 only when the reloaded byte 9 is
 * zero. The work pointer comes from scratchpad slot 0x44 and is reloaded
 * after the calls and after the byte store, which can alias the pointer cell.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndAdvanceStateScenarioScena0100_801F75C8(void) {
  func_801F7678(scenarioScena0100Work[9]);
  updateWorkTableAndReportActiveScenarioScena0100_801F7C54();
  scenarioScena0100Work[9] -= 4;
  if (scenarioScena0100Work[9] == 0) {
    scenarioScena0100Work[1]++;
  }
}
