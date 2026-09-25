#include "bof3/bof3.h"

void func_801C601C(s32 arg0);
/* @source 0x801F2C04
 * @behavior Thin overlay wrapper: calls the helper at 0x801C601C with argument zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeZeroArgWorld00Area00713_801F2C04(void) {
  func_801C601C(0);
}
