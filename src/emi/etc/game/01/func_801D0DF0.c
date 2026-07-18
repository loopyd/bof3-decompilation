#include "internal.h"

/* @behavior decrements the local timer; once it expires, enters local mode `0`,
 * opens the selection FX, and advances the title state.
 * @source 0x801D0DF0
 */
void func_801D0DF0(void) {
  volatile u16* timer_ptr;
  u16           timer;

  timer_ptr = (volatile u16*)0x80140000u;
  timer_ptr += 0x1e10;
  timer = *timer_ptr - 1u;
  *timer_ptr = timer;

  if ((s32)(timer << 0x10) == 0) {
    func_8014ECAC(0);
    func_801D1134();
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
}
