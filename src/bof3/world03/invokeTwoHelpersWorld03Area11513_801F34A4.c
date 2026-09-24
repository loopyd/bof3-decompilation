#include "bof3/bof3.h"

void func_801F34CC(void);
void func_801F3690(void);

/* @source 0x801F34A4
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F34CC and 0x801F3690 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld03Area11513_801F34A4(void) {
  func_801F34CC();
  func_801F3690();
}
