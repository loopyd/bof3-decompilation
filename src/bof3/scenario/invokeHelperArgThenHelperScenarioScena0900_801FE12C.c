#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE12C
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 4 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena0900_801FE12C(void) {
  func_801C1ABC(4);
  func_801C1630();
}
