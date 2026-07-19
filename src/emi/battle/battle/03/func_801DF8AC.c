#include "internal.h"

/* @behavior selects the next local-state handler from one of two tables and calls
 * it immediately.
 * @source 0x801DF8AC
 */
void NO_SIBLING_CALLS func_801DF8AC(void) {
  const volatile u8* state_table_base;
  Battle03Handler    handler;
  u32                state_index;

  state_table_base = BATTLE_HIGH_RAM_U8;

  if ((BATTLE_LOCAL_WORD_128(BATTLE_LOCAL_WORK_PTR) & 1u) != 0u) {
    handler = *(Battle03Handler const volatile*)(state_table_base - 0x4e78u);
  } else {
    state_index = BATTLE_LOCAL_BYTE_79(BATTLE_LOCAL_WORK_PTR);
    handler = *(Battle03Handler const volatile*)((u32)state_table_base +
                                                 (state_index << 2) - 0x4ea4u);
  }

  handler();
}
