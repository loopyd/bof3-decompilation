#include "bof3/ui/game00_internal.h"

/* @behavior clears the first five bytes of the record slot at the given
 * index, zeroing per‑slot fields in the entry table at recordTable.
 * @source 0x801960C0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearRecord(u8 record_index) {
  recordTable[record_index].flags_00 = 0;
  recordTable[record_index].unk_01 = 0;
  recordTable[record_index].unk_02 = 0;
  recordTable[record_index].unk_03 = 0;
  recordTable[record_index].unk_04 = 0;
}
