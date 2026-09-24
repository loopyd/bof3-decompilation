#include "bof3/bof3.h"

void func_801F79C4(void);

/* @source 0x801F7900
 * @behavior Thin overlay wrapper: calls the helper at 0x801F79C4 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperScenarioScena1500_801F7900(void) {
  func_801F79C4();
}
