#include "bof3/bof3.h"

void func_801F2C04(void);

/* @source 0x801F2D2C
 * @behavior Thin overlay wrapper: calls the helper at 0x801F2C04 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWorld01Area07313_801F2D2C(void) {
  func_801F2C04();
}
