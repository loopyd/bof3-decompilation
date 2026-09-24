#include "bof3/bof3.h"

void func_80196070(void);

/* @source 0x801F88DC
 * @behavior Thin overlay wrapper calling the helper at 0x80196070.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperScenarioScena0300_801F88DC(void) {
  func_80196070();
}
