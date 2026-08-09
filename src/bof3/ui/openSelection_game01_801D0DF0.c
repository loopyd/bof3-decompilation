#include "bof3/ui/game01_internal.h"

/* @behavior decrements the local timer; once it expires, enters local mode `0`,
 * opens the selection FX, and advances the title state.
 * @source 0x801D0DF0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void openSelection(void) {
  volatile u16* timer_ptr;
  u16           timer;

  timer_ptr = &GAME_FRONT_TIMER;
  timer = *timer_ptr - 1u;
  *timer_ptr = timer;

  if ((s32)(timer << 0x10) == 0) {
    func_8014ECAC(0);
    startSelectionFx();
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
}
