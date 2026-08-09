#include "bof3/battle/battle03_internal.h"

/* @behavior copies the active-slot table-0 handlers to a local stack table, then
 * dispatches through the current queued-slot byte `5` selector.
 * @source 0x801E5AF4
 * @status partial
 * @match 42.11
 * @residual non-exact live audit: 16/38 instructions; 152 original bytes versus 128 current.
 */
void NO_SIBLING_CALLS func_801E5AF4(void) {
  Battle03Handler table[19];
  u8              index;

  index = 0u;
  do {
    table[index] = BATTLE_ACTIVE_SLOT_TABLE_0[index];
    index += 1u;
  } while (index < 19u);

  table[BATTLE_CURRENT_QUEUED_SLOT_PTR->unk_05]();
}
