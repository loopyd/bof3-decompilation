#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F9B8
 * @behavior Runs the selection-input refresh pass func_800A3F28, then forwards
 *   input mask 0x20 to querySelectionApplyInput.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009F9B8(void) {
  func_800A3F28();
  querySelectionApplyInput(0x20);
}
