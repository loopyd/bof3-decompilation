#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F978
 * @behavior Calls resetSelectionApplyInput with argument 8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009F978(void) {
  resetSelectionApplyInput(0x8);
}
