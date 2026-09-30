#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @calls resetSelectionApplyInput with argument 0x8
 * @source 0x8009F978
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009F978(void) {
  resetSelectionApplyInput(0x8);
}
