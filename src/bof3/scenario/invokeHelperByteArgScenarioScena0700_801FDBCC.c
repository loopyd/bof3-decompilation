#include "bof3/bof3.h"

extern u8 D_80146189;

void func_801C1ABC(s32 arg0);

/* @source 0x801FDBCC
 * @behavior Thin overlay wrapper: passes the shared main-RAM byte D_80146189,
 * loaded unsigned, as the single argument of the helper at 0x801C1ABC and
 * returns. The load is materialised before the stack frame because the argument
 * is its only consumer. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperByteArgScenarioScena0700_801FDBCC(void) {
  func_801C1ABC(D_80146189);
}
