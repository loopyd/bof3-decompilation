#include "bof3/bof3.h"

void func_8014D978(void);

/* @source 0x800C1C68
 * @behavior Thin overlay wrapper: calls the helper at 0x8014D978 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperBossBoss00816_800C1C68(void) {
  func_8014D978();
}
