#include "bof3/bof3.h"

void func_801E5988(void);

/* @source 0x801F007C
 * @behavior Thin overlay wrapper: calls the helper at 0x801E5988 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperBmagicMagic06403_801F007C(void) {
  func_801E5988();
}
