#include "bof3/bof3.h"

void func_801F2C04(void);

/* @source 0x801F2D2C
 * @behavior Thin overlay wrapper calling the helper at 0x801F2C04.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWorld02Area11013_801F2D2C(void) {
  func_801F2C04();
}
