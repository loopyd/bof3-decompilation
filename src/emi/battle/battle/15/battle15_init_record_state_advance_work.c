#include "internal.h"

/* @source 0x800A8450 */
/* @behavior initializes selected D_80145E90 record state fields (01=6, 02/03=4, 04=0) then increments g_battle_work[1]. */
void battle15_init_record_state_advance_work(void) {
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
