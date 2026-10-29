#include "bof3/bof3.h"

void func_801F2E80(s32 arg0);
/* @source 0x801F2CC0
 * @behavior Thin overlay wrapper: calls the helper at 0x801F2E80 with argument zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeZeroArgWorld04Area19913_801F2CC0(void) {
  func_801F2E80(0);
}
