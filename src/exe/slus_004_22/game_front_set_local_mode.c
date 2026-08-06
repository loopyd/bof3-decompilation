#include "internal.h"

/* @behavior stores one frontend-local mode and installs the matching callback.
 * @source 0x8014ECAC
 */
void __attribute__((noinline)) game_front_set_local_mode(u16 local_mode) {
  volatile u16* game_front;

  game_front = (volatile u16*)0x80140000u;

  if (game_front[0x3c40 / sizeof(u16)] != 0u) {
    return;
  }

  game_front[0x3c90 / sizeof(u16)] = local_mode & 0xffu;
  game_install_callback_slot(2, game_front_local_mode_callback_loop);
}
