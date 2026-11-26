#include "bof3/battle/battle15_internal.h"

/* @calls querySelectionRollClamp with argument 1
 * @source 0x8009E7C4
 * @behavior forwards selector 1 to querySelectionRollClamp
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardSelectorOne(void) {
  querySelectionRollClamp(1);
}
