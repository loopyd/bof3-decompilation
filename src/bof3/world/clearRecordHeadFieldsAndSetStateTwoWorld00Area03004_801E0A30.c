#include "bof3/world/area03004_internal.h"

/* @behavior resets the shared 20-slot 0x74-byte front-end entry table at
 * 0x80143FC8: it zeroes head bytes 1-4 of the leading seven record slots
 * through the 0x74-byte table view, clears the remaining slots 7-19 through
 * clearRecord, then stores 2 in the shared state byte D_8014403D.
 * @source 0x801E0A30
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordHeadFieldsAndSetStateTwoWorld00Area03004(void) {
  s32 recordIndex;

  for (recordIndex = 0; recordIndex < 7; recordIndex++) {
    D_80143FC8[recordIndex].unk_01 = 0;
    D_80143FC8[recordIndex].unk_02 = 0;
    D_80143FC8[recordIndex].unk_03 = 0;
    D_80143FC8[recordIndex].unk_04 = 0;
  }
  for (recordIndex = 7; recordIndex < 20; recordIndex++) {
    clearRecord(recordIndex);
  }
  D_8014403D = 2;
}
