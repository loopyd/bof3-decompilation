#include "bof3/bof3.h"

/* @source 0x801F6CFC
 * @behavior Performs no work and touches no state, returning zero to its caller;
 * @status exact
 * @match 100.00
 * @residual none
 * it is the zero-returning no-op code entry that this overlay's callback pointer
 * table references through the word at 0x801F6D60.
 */
s32 noopCallbackReturningZero_scena18(void) {
  return 0;
}
