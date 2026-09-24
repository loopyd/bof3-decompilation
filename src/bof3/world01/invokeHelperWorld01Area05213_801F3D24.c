#include "bof3/bof3.h"

void func_801F431C(void);

/* @source 0x801F3D24
 * @behavior Thin overlay wrapper: calls the helper at 0x801F431C and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWorld01Area05213_801F3D24(void) {
  func_801F431C();
}
