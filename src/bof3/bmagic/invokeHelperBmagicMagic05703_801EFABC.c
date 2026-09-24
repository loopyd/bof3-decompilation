#include "bof3/bof3.h"

void func_801E5988(void);

/* @source 0x801EFABC
 * @behavior Thin overlay wrapper calling the helper at 0x801E5988.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperBmagicMagic05703_801EFABC(void) {
  func_801E5988();
}
