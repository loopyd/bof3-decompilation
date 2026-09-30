#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FD4D8
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC twice, with
 * the constant arguments 5 and then 1, and then the helper at 0x801C1630 with
 * no arguments; takes no arguments and returns nothing. The original bytes
 * prove the shape: a 0x30-byte body whose 0x18-byte frame only keeps $ra across
 * the three calls, two jal operands 0x0C0706AF (0x801C1ABC) each followed in
 * the delay slot by the immediate 0x24040005 (5) and then 0x24040001 (1), and a
 * third jal operand 0x0C07058C (0x801C1630) followed by a nop. The call
 * sequence is the exact sibling invokeHelperArgThenHelperScenarioScena0500_801FD508
 * (0x801FD508) with one extra leading call to 0x801C1ABC, so the name records
 * the proven shape and that family: invoke the helper at 0x801C1ABC with a
 * constant argument twice and then the helper at 0x801C1630. The name follows
 * the established family spelling for this exact three-call shape, e.g. the
 * exact siblings invokeHelperTwiceThenHelperScenarioScena0600_801FE254,
 * invokeHelperTwiceThenHelperScenarioScena0600_801FE284 and
 * invokeHelperTwiceThenHelperScenarioScena0900_801FE154.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperTwiceThenHelperScenarioScena0500_801FD4D8(void) {
  func_801C1ABC(5);
  func_801C1ABC(1);
  func_801C1630();
}
