#include "bof3/bof3.h"

void func_801C601C(s32 arg0);

/* @source 0x801F4230
 * @behavior Thin overlay wrapper calling the helper at 0x801C601C with one constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgWorld01Area04113_801F4230(void) {
  func_801C601C(8);
}
