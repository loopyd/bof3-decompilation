#include "bof3/bof3.h"

void func_80196070(void);

/* @source 0x801F73A4
 * @behavior Thin overlay wrapper: calls the helper at 0x80196070 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperScenarioScena0700_801F73A4(void) {
  func_80196070();
}
