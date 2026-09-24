#include "bof3/bof3.h"

void func_80196070(void);

/* @source 0x801F2E0C
 * @behavior Thin overlay wrapper calling the helper at 0x80196070.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWorld04Area17513_801F2E0C(void) {
  func_80196070();
}
