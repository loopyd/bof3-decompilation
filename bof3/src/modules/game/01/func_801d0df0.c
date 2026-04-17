#include "internal.h"

/* does: decrements the local timer; once it expires, enters local mode `0`,
 * opens the selection FX, and advances the title state.
 * @source: 0x801d0df0 FUN_801d0df0
 * @source: docs/specs/runtime/game-overlay.md
 */
void func_801d0df0(void) {
  u16 timer;

  timer = BOF3_GAME_FRONT_TIMER - 1u;
  BOF3_GAME_FRONT_TIMER = timer;

  if ((s32)(timer << 0x10) == 0) {
    func_8014ecac(0);
    func_801d1134();
    BOF3_GAME_FRONT_STATE = BOF3_GAME_FRONT_STATE + 1u;
  }
}
