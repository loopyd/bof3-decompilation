#include "bof3/bof3.h"

void func_801F6FF8(void);
void func_801F70DC(void);

/* @source 0x801F6FD0
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F6FF8 and 0x801F70DC in sequence.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersScenarioScena1300_801F6FD0(void) {
  func_801F6FF8();
  func_801F70DC();
}
