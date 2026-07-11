#include "internal.h"

/* @behavior stages one selection-ring record for the current grid cursor and then
 * advances the confirm-selection substate byte.
 * @source 0x80096f78 FUN_80096f78
 */
void func_80096f78(void) {
  u32 name_handle;

  name_handle = battle_resolve_frontend_resource(0x4000u);
  battle_stage_selection_ring_record(2u, 0xffu, name_handle);
  BATTLE_SELECTION_SUBSTATE += 1u;
}
