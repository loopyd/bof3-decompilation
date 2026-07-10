#include "internal.h"

/* does: derives one base damage value from the queued base halfword, then
 * adjusts it through the enemy-side variance path before handing it to
 * `func_801dcd50`.
 * @source: 0x801dcad8 FUN_801dcad8
 */
u32 func_801dcad8(u8 arg0, u8 arg1, s8 arg2) {
  u32 battler;
  u32 target;
  s32 value;
  s32 rank;
  s32 random_value;
  u16 enemy_stat;
  u8  divisor;

  battler = arg0;
  target = arg1;

  if (((battler & 0xffu) < 3u) || ((target & 0xffu) >= 3u)) {
    value = BATTLE_GLOBAL_HALF_EC30C;
    if ((u8)arg2 == 0u) {
      value -= BATTLE_GLOBAL_HALF_EC2EE;
    }
    if (value < 0) {
      value = 0;
    }
  } else {
    value = BATTLE_GLOBAL_HALF_EC30C;
    if ((u8)arg2 == 0u) {
      value -= func_801dccb0() & 0xffff;
    }
    if (value < 0) {
      value = 0;
    }

    rank = -(value / 5);
    if (value >= -4) {
      rank = 0;
    }
    if (value < -0x33) {
      rank = 8u;
    }

    random_value = func_8017e3d4();
    enemy_stat =
        *(volatile u16*)((volatile u8*)&BATTLE_ENEMY_WORK_ARRAY[(battler - 3u) &
                                                                0xffu] +
                         0x88u);
    divisor = BATTLE_VARIANCE_TABLE_AF94[rank & 0xffu];
    if (divisor != 0u) {
      value = (value * 0x100 +
               (((enemy_stat * 0x100) * ((random_value % 2) + 2) * 0x100) /
                ((s32)divisor << 8))) >>
              8;
    }
  }

  return func_801dcd50(battler & 0xffu, target & 0xffu,
                       value + (func_8017e3d4() & 1u));
}
