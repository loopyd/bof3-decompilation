#include "internal.h"

/* does: conditionally zeroes one enemy-side `0x10` status bit after the random
 * gates for the stricter enemy-target path, otherwise passing the signed damage
 * value through unchanged.
 * @source: 0x801dc894 FUN_801dc894
 */
u32 func_801dc894(s16 arg0, u8 arg1, u32 arg2) {
  u16 flags;

  if ((BOF3_BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) {
    return (u32)(s32)arg0;
  }

  if (arg1 < 3u) {
    flags = BOF3_BATTLE_LOCAL_FLAGS_80(&BOF3_BATTLE_LOCAL_WORK_ARRAY[arg1]);
    if (((flags & 8u) != 0u) && ((func_8017e3d4() & 2u) != 0u)) {
      BOF3_BATTLE_ENEMY_BYTE_FC(
          &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg2 - 3u) & 0xffu]) &= 0xefu;
      return 0u;
    }
  } else {
    flags = BOF3_BATTLE_ENEMY_FLAGS_82(
        &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]);
    if (((flags & 8u) != 0u) && ((func_8017e3d4() & 2u) != 0u)) {
      BOF3_BATTLE_ENEMY_BYTE_FC(
          &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg2 - 3u) & 0xffu]) &= 0xefu;
      return 0u;
    }
  }

  if ((arg1 >= 3u) ||
      ((BOF3_BATTLE_LOCAL_WORD_128(&BOF3_BATTLE_LOCAL_WORK_ARRAY[arg1]) &
        0x100u) == 0u)) {
    if ((arg1 < 3u) &&
        (BOF3_BATTLE_GLOBAL_BYTE_EC324 <= (func_8017e3d4() % 100))) {
      BOF3_BATTLE_ENEMY_BYTE_FC(
          &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg2 - 3u) & 0xffu]) &= 0xefu;
      return 0u;
    }
  }

  return (u32)(s32)arg0;
}
