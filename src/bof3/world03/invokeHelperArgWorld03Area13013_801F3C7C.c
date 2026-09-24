#include "bof3/bof3.h"

void func_80166180(s32 arg0);

/* @source 0x801F3C7C
 * @behavior Thin overlay wrapper calling the helper at 0x80166180 with one constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgWorld03Area13013_801F3C7C(void) {
  func_80166180(10);
}
