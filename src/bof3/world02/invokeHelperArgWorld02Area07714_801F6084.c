#include "bof3/bof3.h"

void func_801F61F0(s32 arg0);

/* @source 0x801F6084
 * @behavior Thin overlay wrapper: calls the helper at 0x801F61F0 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgWorld02Area07714_801F6084(void) {
  func_801F61F0(74);
}
