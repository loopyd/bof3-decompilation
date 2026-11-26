#include "bof3/battle/battle15_internal.h"

/* @calls querySelectionRollClamp with argument 0
 * @source 0x8009E1E0
 * @behavior forwards selector 0 to querySelectionRollClamp
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardSelectorZero(void) {
  querySelectionRollClamp(0);
}
