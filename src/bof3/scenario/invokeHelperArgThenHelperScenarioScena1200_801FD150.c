#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FD150
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 8 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing. The 0x18-byte frame only keeps $ra
 * across the two calls. The original bytes are instruction-identical to the
 * exact siblings invokeHelperArgThenHelperScenarioScena1200_801FD0B0
 * (0x801FD0B0), invokeHelperArgThenHelperScenarioScena1200_801FD0D8
 * (0x801FD0D8), invokeHelperArgThenHelperScenarioScena1200_801FD100
 * (0x801FD100) and invokeHelperArgThenHelperScenarioScena1200_801FD128
 * (0x801FD128) apart from the constant argument immediate, and both jal
 * operands encode 0x801C1ABC (0x0C0706AF) and 0x801C1630 (0x0C07058C). The
 * name records the proven shape and the four exact siblings above: invoke the
 * helper at 0x801C1ABC with a constant argument and then the helper at
 * 0x801C1630.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena1200_801FD150(void) {
  func_801C1ABC(8);
  func_801C1630();
}
