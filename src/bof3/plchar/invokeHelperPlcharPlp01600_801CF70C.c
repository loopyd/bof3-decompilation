#include "bof3/bof3.h"

void func_8014D978(void);

/* @source 0x801CF70C
 * @behavior Thin overlay wrapper: calls the helper at 0x8014D978 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperPlcharPlp01600_801CF70C(void) {
  func_8014D978();
}
