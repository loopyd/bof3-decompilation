#include "bof3/bof3.h"

void func_80196070(void);

/* @source 0x801D369C
 * @behavior Thin overlay wrapper: calls the helper at 0x80196070 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperScenarioSce15ef100_801D369C(void) {
  func_80196070();
}
