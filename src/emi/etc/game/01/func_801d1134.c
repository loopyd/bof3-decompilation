#include "internal.h"

/* @behavior opens the selection-specific EXE effect using the current selection.
 * @source 0x801D1134
 */
void func_801D1134(void) {
  s32 selection;

  selection = GAME_FRONT_SELECTION;

  if (selection != 0xffu) {
    u32 selection_offset;
    s32 effect_group;
    s32 effect_id;

    selection_offset = selection << 2;
    effect_group = GAME_FRONT_SELECTION_FX_TABLE[selection_offset];
    effect_id = GAME_FRONT_SELECTION_FX_TABLE[selection_offset + 1u];
    game_start_selection_fx(effect_group, effect_id, 100, 0x10);
  }
}
