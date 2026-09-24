#include "bof3/bof3.h"

void func_801F34E0(void);
void func_801F36A4(void);

/* @source 0x801F34B8
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F34E0 and 0x801F36A4 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld03Area12114_801F34B8(void) {
  func_801F34E0();
  func_801F36A4();
}
