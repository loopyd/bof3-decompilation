#include "bof3/battle/battle15_internal.h"

/* @source 0x8009E804
 * @behavior Calls resetSelectionApplyInput at 0x800A41D8 with argument 0x40 and
 * returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetSelectionApplyInput40(void) {
  resetSelectionApplyInput(0x40);
}
