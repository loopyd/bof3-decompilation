#include "internal.h"

/* does: selects the next local-state handler from one of two tables and calls
 * it immediately.
 * @source: 0x801df8ac FUN_801df8ac
 */
void BOF3_NO_SIBLING_CALLS func_801df8ac(void) {
  const volatile u8* state_table_base;
  Battle03Handler    handler;
  u32                state_index;

  state_table_base = (const volatile u8*)0x801f0000u;

  if ((BOF3_BATTLE_LOCAL_WORD_128(BOF3_BATTLE_LOCAL_WORK_PTR) & 1u) != 0u) {
    handler = *(Battle03Handler const volatile*)(state_table_base - 0x4e78u);
  } else {
    state_index = BOF3_BATTLE_LOCAL_BYTE_79(BOF3_BATTLE_LOCAL_WORK_PTR);
    handler = *(Battle03Handler const volatile*)((u32)state_table_base +
                                                 (state_index << 2) - 0x4ea4u);
  }

  handler();
}
