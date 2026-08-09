#include "bof3/battle/battle15_internal.h"

/* @source 0x800A8450
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
/* @behavior initializes selected D_80145E90 record state fields (01=6, 02/03=4, 04=0) then increments g_battle_work[1]. */
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
