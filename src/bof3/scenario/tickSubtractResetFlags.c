#include "bof3/bof3.h"

extern u8* workObjectPointer; /* @source 0x1F800044 @kind unknown */
extern u16 scenarioHalfword800; /* @source 0x800E4C1E @kind unknown */
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
void tickSubtractResetFlags(void) {
  u16 value;
  u8* work;
  u8 countdown;

  value = scenarioHalfword800;
  work = workObjectPointer;
  scenarioHalfword800 = (u16)(value - 24);
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    func_80196070();
  }
}
