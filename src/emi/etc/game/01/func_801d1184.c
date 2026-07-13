#include "internal.h"

/* @behavior closes the current selection-specific EXE effect and clears the active
 * selection byte.
 * @source 0x801d1184 FUN_801d1184
 */
void func_801d1184(void) {
  volatile u8* selection_ptr = &GAME_FRONT_SELECTION;
  s32          selection = *selection_ptr;

  if (selection != 0xffu) {
    s32 selection_offset = selection << 2;

    game_stop_selection_fx(
        GAME_FRONT_SELECTION_FX_TABLE[selection_offset + 0u],
        GAME_FRONT_SELECTION_FX_TABLE[selection_offset + 1u]);
    *selection_ptr = 0xffu;
  }
}
