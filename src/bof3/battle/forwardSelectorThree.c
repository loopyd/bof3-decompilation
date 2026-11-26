#include "bof3/battle/battle15_internal.h"

/* @source 0x8009EF08
 * @behavior Calls querySelectionRollClamp with argument 3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardSelectorThree(void) {
  querySelectionRollClamp(3);
}
