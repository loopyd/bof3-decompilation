#include "bof3/bof3.h"

void func_801F79C4(void);
void func_801F8704(void);

/* @source 0x801F85F0
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F79C4 and 0x801F8704 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersScenarioScena1500_801F85F0(void) {
  func_801F79C4();
  func_801F8704();
}
