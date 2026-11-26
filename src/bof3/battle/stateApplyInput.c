#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F9B8
 * @behavior Runs the selection-input refresh pass stateRefreshFlagStatus, then forwards
 *   input mask 0x20 to querySelectionApplyInput.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateApplyInput(void) {
  stateRefreshFlagStatus();
  querySelectionApplyInput(0x20);
}
