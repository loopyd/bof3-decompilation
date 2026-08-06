#include "internal.h"

extern u8 D_80181EBA[];

/* @behavior opens the selection-specific EXE effect using the current selection.
 * @source 0x801D1134
 */
void startSelectionFx(void) {
  u8 selection;

  selection = GAME_FRONT_SELECTION;

  if (selection != 0xffu) {
    u32 offset = selection << 2;

    game_start_selection_fx(D_80181EBA[offset], D_80181EBA[offset + 1u], 100,
                            0x10);
  }
}
