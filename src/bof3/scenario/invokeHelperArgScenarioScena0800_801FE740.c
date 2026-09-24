#include "bof3/bof3.h"

void func_801C187C(s32 arg0);

/* @source 0x801FE740
 * @behavior Thin overlay wrapper calling the helper at 0x801C187C with one constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgScenarioScena0800_801FE740(void) {
  func_801C187C(2);
}
