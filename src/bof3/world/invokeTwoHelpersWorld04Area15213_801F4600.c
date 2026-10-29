#include "bof3/bof3.h"

void func_8014D978(void);
void func_8014D260(void);

/* @source 0x801F4600
 * @behavior Thin overlay wrapper: calls the helpers at 0x8014D978 and 0x8014D260 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld04Area15213_801F4600(void) {
  func_8014D978();
  func_8014D260();
}
