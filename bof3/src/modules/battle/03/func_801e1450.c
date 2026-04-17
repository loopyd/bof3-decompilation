#include "internal.h"

/* does: dispatches the current alternate local state-3 byte through its table.
 * @source: 0x801e1450 FUN_801e1450
 */
void BOF3_NO_SIBLING_CALLS func_801e1450(void) {
  volatile Battle03LocalWork* work;
  Battle03Handler             handler;
  u32                         state_index;

  work = BOF3_BATTLE_LOCAL_SCRATCH_PTR;
  state_index = work->unk_03;
  handler = *(Battle03Handler const volatile*)((const volatile u8*)0x801f0000u +
                                               (state_index << 2) - 0x4de8u);
  handler();
}
