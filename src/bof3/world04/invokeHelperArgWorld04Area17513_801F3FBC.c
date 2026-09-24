#include "bof3/bof3.h"

void func_8015D71C(s32 arg0);

/* @source 0x801F3FBC
 * @behavior Thin overlay wrapper calling the helper at 0x8015D71C with one constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgWorld04Area17513_801F3FBC(void) {
  func_8015D71C(513);
}
