#include "bof3/bof3.h"

void func_80196070(void);

/* @source 0x801F38C0
 * @behavior Thin overlay wrapper: calls the helper at 0x80196070 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWorld04Area18813_801F38C0(void) {
  func_80196070();
}
