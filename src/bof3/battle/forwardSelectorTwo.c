#include "bof3/battle/battle15_internal.h"

/* @source 0x8009E7E4
 * @behavior Calls querySelectionRollClamp with argument 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardSelectorTwo(void) {
  querySelectionRollClamp(2);
}
