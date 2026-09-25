#include "bof3/bof3.h"

void func_801C187C(s32 arg0);
/* @source 0x801FD36C
 * @behavior Thin overlay wrapper: calls the helper at 0x801C187C with argument zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeZeroArgScenarioScena0500_801FD36C(void) {
  func_801C187C(0);
}
