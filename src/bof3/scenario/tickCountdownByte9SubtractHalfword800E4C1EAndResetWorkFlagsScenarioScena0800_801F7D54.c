#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
extern u16 scenarioScena0800Halfword800E4C1E; /* @source 0x800E4C1E @kind unknown */
void func_80196070(void);

/* @source 0x801F7D54
 * @behavior Subtract 24 from the halfword at 0x800E4C1E modulo 65536,
 * then decrement work byte 9 modulo 256. On countdown expiry, call the
 * work-flag reset helper at 0x80196070. Read the scratchpad work-pointer
 * cell at 0x1F800044 before the halfword store and retain that pointer for
 * the countdown access. Takes no arguments and returns nothing. The
 * halfword's wider role remains unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownByte9SubtractHalfword800E4C1EAndResetWorkFlagsScenarioScena0800_801F7D54(void) {
  u16 value;
  u8* work;
  u8 countdown;

  value = scenarioScena0800Halfword800E4C1E;
  work = scenarioScena0800Work;
  scenarioScena0800Halfword800E4C1E = (u16)(value - 24);
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    func_80196070();
  }
}
