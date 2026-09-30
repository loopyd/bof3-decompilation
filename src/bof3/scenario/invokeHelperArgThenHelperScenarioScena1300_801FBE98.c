#include "bof3/bof3.h"

void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FBE98
 * @behavior Thin overlay wrapper calling the helper at 0x801C187C with the
 * constant argument 4 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena1300_801FBE98(void) {
  func_801C187C(4);
  func_801C1630();
}
