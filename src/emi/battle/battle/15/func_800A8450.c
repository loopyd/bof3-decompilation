#include "internal.h"

/* @behavior initializes the selected local battle work record and advances work state. */
void func_800A8450(void) {
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
