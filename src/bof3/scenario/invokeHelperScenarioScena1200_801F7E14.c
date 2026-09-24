#include "bof3/bof3.h"

void func_80196070(void);

/* @source 0x801F7E14
 * @behavior Thin overlay wrapper: calls the helper at 0x80196070 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperScenarioScena1200_801F7E14(void) {
  func_80196070();
}
