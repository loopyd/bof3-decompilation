#include "internal.h"

extern int rand(void);
/* @behavior conditionally zeroes one local status bit after a random gate,
 * otherwise passing through the signed damage value unchanged.
 * @source 0x801DC73C
 */
u32 func_801DC73C(s16 arg0, u32 arg1, u32 arg2) {
  u16 flags;
  u8  threshold;
  u32 enemy;

  if ((BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) {
    return (u32)(s32)arg0;
  }

  arg1 &= 0xffu;
  if (arg1 < 3u) {
    flags = BATTLE_LOCAL_ABS_HALF_5F10(arg1);
  } else {
    enemy = arg1 - 3u;
    flags = BATTLE_ENEMY_ABS_HALF_6B2(enemy);
  }

  if ((flags & 8u) != 0u) {
    if ((rand() & 2u) != 0u) {
      goto clear_flag;
    }
  }

  threshold = BATTLE_GLOBAL_BYTE_C303;
  if (threshold < (rand() % 100)) {
    return (u32)(s32)arg0;
  }

clear_flag:
  BATTLE_LOCAL_ABS_BYTE_5FB0(arg2) &= 0xefu;
  return 0u;
}
