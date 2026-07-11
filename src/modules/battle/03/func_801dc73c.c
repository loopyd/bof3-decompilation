#include "internal.h"

/* @behavior conditionally zeroes one local status bit after a random gate,
 * otherwise passing through the signed damage value unchanged.
 * @source 0x801dc73c FUN_801dc73c
 */
u32 func_801dc73c(s16 arg0, u32 arg1, u32 arg2) {
  u16 flags;
  u8  threshold;
  u32 enemy;

  if ((*(volatile u16*)(0x80140000u + 0x62e8u) & 0x80u) != 0u) {
    return (u32)(s32)arg0;
  }

  arg1 &= 0xffu;
  if (arg1 < 3u) {
    flags = *(volatile u16*)(0x80140000u + ((arg1 * 5u) << 6) + 0x5f10u);
  } else {
    enemy = arg1 - 3u;
    flags = *(volatile u16*)(0x801f0000u +
                             (((((enemy << 3) + enemy) << 2) - enemy) << 3) -
                             0x494eu);
  }

  if ((flags & 8u) != 0u) {
    if ((func_8017e3d4() & 2u) != 0u) {
      goto clear_flag;
    }
  }

  threshold = *(volatile u8*)(0x801f0000u - 0x3cfdu);
  if (threshold < (func_8017e3d4() % 100)) {
    return (u32)(s32)arg0;
  }

clear_flag:
  *(volatile u8*)(0x80140000u + (((arg2 & 0xffu) * 5u) << 6) + 0x5fb0u) &=
      0xefu;
  return 0u;
}
