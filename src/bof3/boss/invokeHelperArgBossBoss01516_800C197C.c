#include "bof3/bof3.h"

void func_800AC934(s32 arg0);

/* @source 0x800C197C
 * @behavior Thin overlay wrapper calling the helper at 0x800AC934.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgBossBoss01516_800C197C(void) {
  func_800AC934(1);
}
