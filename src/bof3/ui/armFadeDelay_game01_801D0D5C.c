#include "bof3/ui/game01_internal.h"

/* @behavior when fade phase `2` is reached, arms a 360-tick delay and advances
 * the frontend state.
 * @source 0x801D0D5C
 * @status exact
 * @match 100.00
 * @residual none
 */
void armFadeDelay(void) {
  /* Read the phase through a non-volatile view so cc1 folds the zero-extension
   * into the `lbu` instead of emitting `andi 0xff` before the compare (same
   * view as updateBanner's dispatch-phase read). */
  u8 phase = *(u8*)&GAME_FRONT_FADE_PHASE;

  if (phase == 2u) {
    u16 state;
    u16 timer = 360u;
    state = GAME_FRONT_STATE;
    GAME_FRONT_TIMER = timer;
    GAME_FRONT_STATE = state + 1u;
  }
}
