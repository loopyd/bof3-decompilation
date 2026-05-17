#include "internal.h"

/* does: copies the active-slot table-0 handlers to a local stack table, then
 * dispatches through the current queued-slot byte `5` selector.
 * @source: 0x801e5af4 FUN_801e5af4
 */
void NO_SIBLING_CALLS func_801e5af4(void) {
  Battle03Handler table[19];
  u8              index;

  index = 0u;
  do {
    table[index] = BATTLE_ACTIVE_SLOT_TABLE_0[index];
    index += 1u;
  } while (index < 19u);

  table[BATTLE_CURRENT_QUEUED_SLOT_PTR->unk_05]();
}
