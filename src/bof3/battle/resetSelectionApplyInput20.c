#include "bof3/battle/battle15_internal.h"

/* @source 0x8009E824
 * @behavior Calls resetSelectionApplyInput at 0x800A41D8 with argument 0x20 and
 * returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetSelectionApplyInput20(void) {
  resetSelectionApplyInput(0x20);
}
