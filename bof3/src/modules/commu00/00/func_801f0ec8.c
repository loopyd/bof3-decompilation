#include "internal.h"

asm(".weak _COMMU00_TASK_SLOTS\n.set _COMMU00_TASK_SLOTS, 0x80146888\n");
extern volatile Commu00TaskSlot _COMMU00_TASK_SLOTS[];

/* does: applies the type-7 record-specific task-slot variant.
 * @source: 0x801f0ec8
 */
void func_801f0ec8(u8 task_index) {
  COMMU00_SCRATCH_SLOT->active = 1u;
  _COMMU00_TASK_SLOTS[task_index].label_id = 0xc003u;
}
