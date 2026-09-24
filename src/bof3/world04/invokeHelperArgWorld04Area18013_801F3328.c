#include "bof3/bof3.h"

void func_8015D71C(s32 arg0);

/* @source 0x801F3328
 * @behavior Thin overlay wrapper: calls the helper at 0x8015D71C and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgWorld04Area18013_801F3328(void) {
  func_8015D71C(513);
}
