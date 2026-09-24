#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);

/* @source 0x801FE874
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with one constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgScenarioScena0800_801FE874(void) {
  func_801C1ABC(2);
}
