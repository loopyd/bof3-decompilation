#include "internal.h"

/* @behavior opens the selection-specific EXE effect using the current selection.
 * @source 0x801d1134 FUN_801d1134
 */
void __attribute__((noinline)) func_801d1134(void) {
  u32 selection = (u32)GAME_FRONT_SELECTION;

  if (selection != 0xffu) {
    u32 selection_offset = selection << 2;

    game_start_selection_fx(
        GAME_FRONT_SELECTION_FX_TABLE[selection_offset + 0u],
        GAME_FRONT_SELECTION_FX_TABLE[selection_offset + 1u], 100, 0x10);
  }
}
