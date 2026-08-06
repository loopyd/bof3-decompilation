#include "internal.h"

/* @behavior initializes both display/draw environment pairs used by the
 * boot-side double buffer.
 * @source 0x8014AE08
 */
void boot_init_display_envs(void) {
  u8* work;

  work = D_80143D48 + 0x14;
  SetDefDrawEnv((DRAWENV*)work, 0, 0, 0x140, 0xf0);
  SetDefDispEnv((DISPENV*)(work - 0x14), 0, 0xf0, 0x140, 0xf0);
  SetDefDrawEnv((DRAWENV*)(work + 0x90), 0, 0xf0, 0x140, 0xf0);
  SetDefDispEnv((DISPENV*)(work + 0x7c), 0, 0, 0x140, 0xf0);
}
