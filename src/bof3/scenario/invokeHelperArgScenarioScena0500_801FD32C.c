#include "bof3/bof3.h"

void func_801C187C(s32 arg0);

/* @source 0x801FD32C
 * @behavior Thin overlay wrapper calling the helper at 0x801C187C with one constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgScenarioScena0500_801FD32C(void) {
  func_801C187C(5);
}
