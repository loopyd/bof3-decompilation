#include "bof3/ui/game01_internal.h"

/* @behavior counts down the frontend timer; at zero it selects fade phase `3`,
 * opens the window phase, rearms 900 ticks, and advances the state.
 * @source 0x801D0D94
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void tickFadeTimer(void) {
  u16* timer_ptr;
  u16  timer;

  timer_ptr = &GAME_FRONT_TIMER;
  timer = *timer_ptr - 1u;
  *timer_ptr = timer;

  if ((s32)(timer << 16) == 0) {
    GAME_FRONT_FADE_PHASE = 3u;
    GAME_FRONT_WINDOW_PHASE = 1u;
    *timer_ptr = 900u;
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
}
