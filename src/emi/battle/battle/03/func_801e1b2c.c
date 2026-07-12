#include "internal.h"

/* @behavior conditionally runs the current scratch-object reset helper when global
 * bit `0x4` is set.
 * @source 0x801e1b2c FUN_801e1b2c
 */
void func_801e1b2c(void) {
  volatile u8* battle_global_base;

  battle_global_base = (volatile u8*)0x80140000u;
  if ((*(volatile u16*)(battle_global_base + 0x62e8u) & 4u) == 0u) {
    return;
  }

  func_801e1dd4();
}
