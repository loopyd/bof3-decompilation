#include "bof3/bof3.h"

void func_801F2DF4(s32 arg0);
void func_801F2ED8(void);

/* @source 0x801F2D20
 * @behavior Thin overlay wrapper calling the helper at 0x801F2DF4 with the
 * constant argument 2 and then the helper at 0x801F2ED8 with no arguments;
 * takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperWorld04Area17213_801F2D20(void) {
  func_801F2DF4(2);
  func_801F2ED8();
}
