#include "bof3/bof3.h"

void func_801F2DE0(void);

/* @source 0x801F2CB4
 * @behavior Thin overlay wrapper calling the helper at 0x801F2DE0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWorld03Area14913_801F2CB4(void) {
  func_801F2DE0();
}
