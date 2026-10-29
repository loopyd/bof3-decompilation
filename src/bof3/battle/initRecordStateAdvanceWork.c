#include "bof3/battle/battle15_internal.h"

/* @source 0x800A8450
 * @behavior For each store, rereads byte 0x80146374 to select a 0x140-byte record
 * at 0x80145E90. Writes 6 at +0x01, 4 at +0x02 and +0x03, and 0 at +0x04, then
 * increments byte +0x01 through the pointer at 0x1F800044 modulo 256.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initRecordStateAdvanceWork(void) {
  volatile u8* selection_index;
  u8           value;

  selection_index = &D_80146374;
  D_80145E90[*selection_index].unk_01 = 6u;
  value = 4u;
  D_80145E90[*selection_index].unk_02 = value;
  D_80145E90[*selection_index].unk_03 = value;
  D_80145E90[*selection_index].unk_04 = 0u;
  g_battle_work[1]++;
}
