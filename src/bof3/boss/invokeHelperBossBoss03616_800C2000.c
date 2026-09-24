#include "bof3/bof3.h"

void func_801E5988(void);

/* @source 0x800C2000
 * @behavior Thin overlay wrapper: calls the helper at 0x801E5988 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperBossBoss03616_800C2000(void) {
  func_801E5988();
}
