#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F7CF8
 * @behavior Decrement the scratchpad work object's byte-9 countdown modulo
 * 256. When it reaches zero, reset it to 8 and increment work byte 2 modulo
 * 256. Read the work-pointer cell at 0x1F800044 again after each byte store
 * on the expiry path, as the original does. This leaf takes no arguments,
 * makes no calls and has no return value or stack frame. The field names
 * remain offset-based because their wider roles are not established.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownByte9AndAdvanceWorkByte2ScenarioScena0800_801F7CF8(void) {
  u8 countdown;

  countdown = (u8)(scenarioScena0800Work[9] - 1);
  scenarioScena0800Work[9] = countdown;
  if (countdown == 0) {
    scenarioScena0800Work[9] = 8;
    scenarioScena0800Work[2]++;
  }
}
