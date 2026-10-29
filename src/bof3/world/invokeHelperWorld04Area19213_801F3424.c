#include "bof3/bof3.h"

void func_801F3444(void);

/* @source 0x801F3424
 * @behavior Thin overlay wrapper: calls the helper at 0x801F3444 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWorld04Area19213_801F3424(void) {
  func_801F3444();
}
