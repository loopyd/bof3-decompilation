#include "internal.h"

extern u8 D_80181EBA[];

/* @behavior closes the current selection-specific EXE effect and clears the active
 * selection byte.
 * @source 0x801D1184
 */
void stopSelectionFx(void) {
  u8* selection_ptr = &GAME_FRONT_SELECTION;
  u8  selection = *selection_ptr;

  if (selection != 0xffu) {
    u32 offset = selection << 2;

    game_stop_selection_fx(D_80181EBA[offset], D_80181EBA[offset + 1u]);
    *selection_ptr = 0xffu;
  }
}
