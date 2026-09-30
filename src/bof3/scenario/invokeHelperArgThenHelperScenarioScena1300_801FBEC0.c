#include "bof3/bof3.h"

void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FBEC0
 * @behavior Thin overlay wrapper calling the helper at 0x801C187C with the
 * constant argument 2 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena1300_801FBEC0(void) {
  func_801C187C(2);
  func_801C1630();
}
