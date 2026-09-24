#include "bof3/bof3.h"

void func_801C601C(s32 arg0);

/* @source 0x801F2C44
 * @behavior Thin overlay wrapper calling the helper at 0x801C601C with one constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgWorld00Area00713_801F2C44(void) {
  func_801C601C(2);
}
