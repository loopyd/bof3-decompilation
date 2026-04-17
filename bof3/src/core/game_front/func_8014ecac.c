#include "internal.h"

/* does: stores one frontend-local mode and installs the matching callback.
 * @source: 0x8014ecac
 */
void __attribute__((noinline)) func_8014ecac(u16 local_mode) {
  volatile u16* game_front;

  game_front = (volatile u16*)0x80140000u;

  if (game_front[0x3c40 / sizeof(u16)] != 0u) {
    return;
  }

  game_front[0x3c90 / sizeof(u16)] = local_mode & 0xffu;
  func_8014b854(2, game_front_local_mode_callback_loop);
}
