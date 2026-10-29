#include "bof3/bof3.h"

void func_801F338C(void);
void func_801F3550(void);

/* @source 0x801F3364
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F338C and 0x801F3550 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld04Area15213_801F3364(void) {
  func_801F338C();
  func_801F3550();
}
