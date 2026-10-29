#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F258
 * @behavior Calls resetSelectionApplyInput with argument 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009F258(void) {
  resetSelectionApplyInput(0x10);
}
