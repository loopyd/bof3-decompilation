#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F998
 * @behavior Calls resetSelectionApplyInput with argument 0x80.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009F998(void) {
  resetSelectionApplyInput(0x80);
}
