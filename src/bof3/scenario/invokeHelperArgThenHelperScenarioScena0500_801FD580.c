#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FD580
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 0 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing. The original bytes prove the shape:
 * a 0x28-byte body whose 0x18-byte frame only keeps $ra across the two calls,
 * the first jal operand 0x0C0706AF (0x801C1ABC) followed by 0x00002021
 * (addu $a0, $zero, $zero, i.e. the argument 0 placed in its delay slot), and
 * the second jal operand 0x0C07058C (0x801C1630) followed by a nop. Apart from
 * that delay-slot instruction the bytes are instruction-identical to the exact
 * siblings invokeHelperArgThenHelperScenarioScena0500_801FD508 (0x801FD508,
 * constant 1), invokeHelperArgThenHelperScenarioScena0500_801FD530
 * (0x801FD530, constant 5) and invokeHelperArgThenHelperScenarioScena0500_801FD558
 * (0x801FD558, constant 6), so the name records the proven shape and that
 * family: invoke the helper at 0x801C1ABC with a constant argument and then the
 * helper at 0x801C1630.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena0500_801FD580(void) {
  func_801C1ABC(0);
  func_801C1630();
}
