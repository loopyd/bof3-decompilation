#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F864
 * @behavior Calls resetSelectionApplyInput with argument 4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetSelectionApplyInput4(void) {
  resetSelectionApplyInput(0x4);
}
