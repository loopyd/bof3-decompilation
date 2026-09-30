#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @calls resetSelectionApplyInput with argument 0x20
 * @source 0x8009E824
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetSelectionApplyInput20(void) {
  resetSelectionApplyInput(0x20);
}
