#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F7C98
 * @behavior Decrement work byte 9 modulo 256 and subtract 0x10000 from
 * work word 0x38 modulo 2^32. On countdown expiry, reset byte 9 to 0x80
 * and increment work byte 2 modulo 256. Reload the scratchpad work-pointer
 * cell after each expiry-path store, as the original does. This leaf has
 * no arguments, calls, stack frame or return value. Field roles remain
 * offset-anchored because their wider meanings are unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownByte9SubtractWorkWord38AndAdvanceWorkByte2ScenarioScena0800_801F7C98(void) {
  u8* work;
  u8 countdown;

  work = scenarioScena0800Work;
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  *(u32*)(work + 0x38) -= 0x10000u;
  if (countdown == 0) {
    scenarioScena0800Work[9] = 0x80;
    scenarioScena0800Work[2]++;
  }
}
