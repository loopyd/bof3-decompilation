#include "bof3/bof3.h"

void func_801F3438(void);
void func_801F35FC(void);

/* @source 0x801F3410
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F3438 and 0x801F35FC in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld03Area15113_801F3410(void) {
  func_801F3438();
  func_801F35FC();
}
