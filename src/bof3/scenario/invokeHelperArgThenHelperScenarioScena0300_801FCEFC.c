#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FCEFC
 * @behavior Entry 3 of the overlay's 4-pointer helper table at 0x801FD038
 * ({0x801FCE18, 0x801FCE6C, 0x801FCEDC, 0x801FCEFC}): a thin wrapper that
 * calls the helper at 0x801C1ABC with the constant argument 1 and then the
 * helper at 0x801C1630 with no arguments; takes no arguments and returns
 * nothing. Byte-identical to the already-lifted sibling
 * invokeHelperArgThenHelperScenarioScena0500_801FD508.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena0300_801FCEFC(void) {
  func_801C1ABC(1);
  func_801C1630();
}
