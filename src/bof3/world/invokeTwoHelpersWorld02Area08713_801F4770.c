#include "bof3/bof3.h"

void func_8014D978(void);
void func_8014D260(void);

/* @source 0x801F4770
 * @behavior Thin overlay wrapper: calls the helpers at 0x8014D978 and 0x8014D260 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld02Area08713_801F4770(void) {
  func_8014D978();
  func_8014D260();
}
