#include "bof3/bof3.h"

s32 func_801F75BC(void);
void func_80196070(void);

/* @source 0x801F73F0
 * @behavior Entry 1 of the two-word overlay handler table at 0x801FE384
 * (entry 0 is func_801F73A0), which the overlay's frame dispatcher
 * func_801F735C selects with byte 1 of the scratchpad work object at
 * 0x1F800044. It runs the record-bank sweep func_801F75BC - which walks the
 * 0x20 records of 0x28-byte stride in the 0x800E5000 bank chosen by that same
 * work object's byte 6, ages each record whose lead byte at offset 0 is
 * non-zero and returns non-zero once it has handled at least one - and, when
 * that result masked to its low byte is zero, resets the work object's state
 * through the shared func_80196070 helper. Takes no arguments and returns
 * nothing; the 0x18-byte frame only keeps $ra across the two calls and the
 * second call is reached only on the zero path. The prototype below is the
 * caller-side width: only the low byte of the result is consumed. The 0x34
 * bytes are the same shape as the exact sibling
 * resetWorkStateWhenNoEntryClearedScenarioScena0100_801F7644 (0x801F7644).
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkStateWhenNoRecordActiveScenarioScena0900_801F73F0(void) {
  if ((func_801F75BC() & 0xFF) == 0) {
    func_80196070();
  }
}
