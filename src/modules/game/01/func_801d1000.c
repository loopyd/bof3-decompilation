#include "internal.h"

/* possible name: game_state_finalize_exit
 * @behavior closes the current selection FX, restores layout bank `0`, installs
 * the alternate frontend callback loop, then exits the active EXE callback
 * thread.
 * @source 0x801d1000 FUN_801d1000
 */
void func_801d1000(void) {
  volatile u16* effect_busy;

  effect_busy = (volatile u16*)0x80140000u;
  if (effect_busy[0x1e20] != 0u) {
    return;
  }

  func_801d1184();
  game_set_frontend_layout_bank(0u);
  func_8014b854(0, func_80196f78);
  func_8014b8b0();
}
