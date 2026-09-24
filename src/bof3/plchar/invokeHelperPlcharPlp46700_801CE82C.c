#include "bof3/bof3.h"

void func_8014D978(void);

/* @source 0x801CE82C
 * @behavior Thin overlay wrapper: calls the helper at 0x8014D978 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperPlcharPlp46700_801CE82C(void) {
  func_8014D978();
}
