#include "bof3/battle/battle15_internal.h"

/* @source 0x800A23CC
 * @behavior Runs the selection-input refresh pass func_800A3F28, then applies
 *   the 0x20, 0x80 and 0x8 input masks in turn through
 *   querySelectionApplyInput.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A23CC(void) {
  func_800A3F28();
  querySelectionApplyInput(0x20);
  querySelectionApplyInput(0x80);
  querySelectionApplyInput(0x8);
}
