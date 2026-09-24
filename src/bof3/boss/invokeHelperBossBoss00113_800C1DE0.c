#include "bof3/bof3.h"

void func_8014D978(void);

/* @source 0x800C1DE0
 * @behavior Thin overlay wrapper: calls the helper at 0x8014D978 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperBossBoss00113_800C1DE0(void) {
  func_8014D978();
}
