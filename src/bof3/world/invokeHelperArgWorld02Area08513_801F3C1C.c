#include "bof3/bof3.h"

void func_801F3C3C(s32 arg0);

/* @source 0x801F3C1C
 * @behavior Thin overlay wrapper calling the helper at 0x801F3C3C with one constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgWorld02Area08513_801F3C1C(void) {
  func_801F3C3C(-8);
}
