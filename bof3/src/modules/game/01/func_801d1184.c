#include "internal.h"

/* does: closes the current selection-specific EXE effect and clears the active
 * selection byte.
 * @source: 0x801d1184 FUN_801d1184
 * @source: docs/specs/runtime/game-overlay.md
 */
void __attribute__((noinline)) func_801d1184(void) {
  u32 selection = (u32)BOF3_GAME_FRONT_SELECTION;

  if (selection != 0xffu) {
    u32 selection_offset = selection << 2;

    game_stop_selection_fx(
        BOF3_GAME_FRONT_SELECTION_FX_TABLE[selection_offset + 0u],
        BOF3_GAME_FRONT_SELECTION_FX_TABLE[selection_offset + 1u]);
    BOF3_GAME_FRONT_SELECTION = 0xffu;
  }
}
