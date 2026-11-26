#include "bof3/battle/battle15_internal.h"

/* @source 0x800A0A04
 * @behavior Runs the selection-input refresh pass stateRefreshFlagStatus, adds 10 to the
 *   clamped byte +0x14 of the record at index 4 of the active selection record
 *   pointer, then forwards the active battler index byte D_80146394 to
 *   func_800A9BD8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateClampAdd10(void) {
  stateRefreshFlagStatus();
  clampIndexedBattleByte14(0xA, 4);
  func_800A9BD8(D_80146394);
}
