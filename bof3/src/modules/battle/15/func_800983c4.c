#include "internal.h"

/* does: stages one secondary selection-ring record through the shared
 * frontend handle path, then advances the secondary substate byte.
 * @source: 0x800983c4 FUN_800983c4
 */
void func_800983c4(void) {
  u32 name_handle;

  name_handle = battle_resolve_frontend_resource(0x4000u);
  battle_stage_selection_ring_record(2u, 0xffu, name_handle);
  BATTLE_SELECTION_SUBSTATE += 1u;
}
