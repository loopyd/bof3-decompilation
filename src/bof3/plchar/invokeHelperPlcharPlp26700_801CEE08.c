#include "bof3/bof3.h"

void func_8014D978(void);

/* @source 0x801CEE08
 * @behavior Thin overlay wrapper: calls the helper at 0x8014D978 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperPlcharPlp26700_801CEE08(void) {
  func_8014D978();
}
