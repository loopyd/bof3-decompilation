#include "bof3/bof3.h"

void func_80196070(void);

/* @source 0x801F2D80
 * @behavior Thin overlay wrapper calling the helper at 0x80196070.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWorld02Area11113_801F2D80(void) {
  func_80196070();
}
