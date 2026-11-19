#include "bof3/world/area03004_internal.h"

/* @behavior Attempts to consume one unit of the entry ID in byte 0x80145026
 * through func_801665A0(3, entry ID, 1, 0). A zero low-byte result
 * clears bytes 0x80145026 and 0x80145027. Always runs
 * confirmSelectionStep afterward.
 * @source 0x801DC4B8
 * @status exact
 * @match 100.00
 * @residual none
 */
void consumeEntryUnitOrClearPairedBytesThenConfirmSelectionStepWorld00Area03004(void) {
  u8* entryId = &D_80145026;

  if (func_801665A0(3, *entryId, 1, 0) == 0) {
    *entryId = 0;
    D_80145027 = 0;
  }
  confirmSelectionStep();
}
