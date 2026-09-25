#include "bof3/bof3.h"

void func_800AC844(s32 arg0);
/* @source 0x800C1EE8
 * @behavior Thin overlay wrapper: calls the helper at 0x800AC844 with argument zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeZeroArgBossBoss00216_800C1EE8(void) {
  func_800AC844(0);
}
