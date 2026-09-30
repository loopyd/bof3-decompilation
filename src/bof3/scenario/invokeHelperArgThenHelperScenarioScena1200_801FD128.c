#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FD128
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 5 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing. The 0x18-byte frame only keeps $ra
 * across the two calls. The original bytes are instruction-identical to the
 * exact siblings invokeHelperArgThenHelperScenarioScena1200_801FD0B0
 * (0x801FD0B0) and invokeHelperArgThenHelperScenarioScena1200_801FD0D8
 * (0x801FD0D8) apart from the constant argument immediate, and both jal
 * operands encode 0x801C1ABC (0x0C0706AF) and 0x801C1630 (0x0C07058C).
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena1200_801FD128(void) {
  func_801C1ABC(5);
  func_801C1630();
}
