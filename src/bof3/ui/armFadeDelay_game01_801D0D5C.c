#include "bof3/ui/game01_internal.h"

/* @behavior when fade phase `2` is reached, arms a 360-tick delay and advances
 * the frontend state.
 * @source 0x801D0D5C
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void armFadeDelay(void) {

  if (*(u8*)&GAME_FRONT_FADE_PHASE == 2u) {

    u16 state;
    u16 timer = 360u;
    state = GAME_FRONT_STATE;
    GAME_FRONT_TIMER = timer;
    GAME_FRONT_STATE = state + 1u;
  }
}
