#include "internal.h"

/* @behavior closes the current selection FX, restores layout bank `0`, installs
 * the alternate frontend callback loop, then exits the active EXE callback
 * thread.
 * @source 0x801D1000
 */
void game_front_finalize_exit(void) {
  volatile u16* effect_busy;

  effect_busy = (volatile u16*)0x80140000u;
  if (effect_busy[0x1e20] != 0u) {
    return;
  }

  game_front_stop_selection_fx();
  func_80161808(0u);
  func_8014B854(0, func_80196F78);
  func_8014B8B0();
}
