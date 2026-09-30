#include "bof3/bof3.h"

void func_801F4E8C(s32 arg0, s32 arg1);

/* @source 0x801F4E68
 * @behavior Thin overlay wrapper calling the helper at 0x801F4E8C with the
 * constant arguments 0xDC and 0xC8; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgsWorld04Area17213_801F4E68(void) {
  func_801F4E8C(0xDC, 0xC8);
}
