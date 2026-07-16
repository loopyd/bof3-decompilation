#include "internal.h"

/* @behavior when fade phase `2` is reached, arms a 360-tick delay and advances
 * the frontend state.
 * @source 0x801D0D5C
 */
void func_801D0D5C(void) {
  if (*(volatile unsigned char*)0x80143c31u == 2) {
    GAME_FRONT_TIMER = 360u;
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
}
