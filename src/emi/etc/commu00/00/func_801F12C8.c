#include "internal.h"

/**
 * @source 0x801F12C8
 * @behavior Copies the selected record-kind byte into the task-indexed UI record.
 */
u8 func_801F12C8(Commu00TaskSlot *slot)
{
  u32 task_index;
  s32 stride;

  task_index = slot->variant_arg_1;
  stride = 8;
  D_801CA28C[(task_index * 24) - task_index] =
      D_801457AB[(D_801455C9[slot->source_index * stride] - 1) * stride];
  return 0;
}
