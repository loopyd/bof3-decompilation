#include "bof3/bof3.h"

void func_801F79C4(void);
void func_801F8958(void);

/* @source 0x801F8930
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F79C4 and 0x801F8958 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersScenarioScena1500_801F8930(void) {
  func_801F79C4();
  func_801F8958();
}
