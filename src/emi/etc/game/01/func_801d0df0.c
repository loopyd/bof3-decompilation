#include "internal.h"

/* @behavior decrements the local timer; once it expires, enters local mode `0`,
 * opens the selection FX, and advances the title state.
 * @source 0x801d0df0 FUN_801d0df0
 */
void func_801d0df0(void) {
  u16 timer;

  timer = GAME_FRONT_TIMER - 1u;
  GAME_FRONT_TIMER = timer;

  if ((s32)(timer << 0x10) == 0) {
    func_8014ecac(0);
    func_801d1134();
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
}
