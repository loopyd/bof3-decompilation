#include "bof3/bof3.h"

void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE49C
 * @behavior Thin overlay wrapper: calls the helper at 0x801C187C with the
 * constant argument 4, then the same helper with the constant argument 2, and
 * finally the helper at 0x801C1630 with no arguments; it takes no arguments,
 * returns nothing and touches no state of its own. The 0x18-byte frame only
 * keeps $ra across the three calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperTwiceThenHelperScenarioScena1500_801FE49C(void) {
  func_801C187C(4);
  func_801C187C(2);
  func_801C1630();
}
