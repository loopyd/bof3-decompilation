#include "bof3/bof3.h"

void func_80196070(void);

/* @source 0x801F2F08
 * @behavior Thin overlay wrapper calling the helper at 0x80196070.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWorld02Area08813_801F2F08(void) {
  func_80196070();
}
