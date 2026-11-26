#include "bof3/bof3.h"

void func_801F338C(void);
void dispatchStateByte3Table4(void);

/* @source 0x801F3364
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F338C and 0x801F3550 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpers(void) {
  func_801F338C();
  dispatchStateByte3Table4();
}
