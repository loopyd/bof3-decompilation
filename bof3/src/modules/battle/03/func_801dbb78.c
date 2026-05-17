#include "internal.h"

/* does: applies one signed damage packet to a local or enemy target, updates
 * current values and followup flags, and runs the local special-case hooks for
 * certain source states.
 * @source: 0x801dbb78 FUN_801dbb78
 */
u32 func_801dbb78(u8 arg0, u8 arg1) {
  volatile Battle03LocalWork* local_work;
  volatile Battle03EnemyWork* enemy_work;
  s16                         value;

  if (arg1 < 3u) {
    BATTLE_LOCAL_BYTE_120(&BATTLE_LOCAL_WORK_ARRAY[arg1]) = 0x11u;
  } else {
    BATTLE_ENEMY_BYTE_FC(
        &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]) = 0x11u;
  }

  value = (s16)func_801dc044(arg0, arg1, 0xffffu);
  if (arg1 < 3u) {
    local_work = &BATTLE_LOCAL_WORK_ARRAY[arg1];
    if ((BATTLE_LOCAL_WORD_134(local_work) & 0x100u) != 0u) {
      value = BATTLE_LOCAL_HALF_88(local_work);
      func_801de560(1u, 0u, 0u, 0x1eu, 0x801eb075u);
      BATTLE_UI_BYTE_83C3 = 1u;
      BATTLE_LOCAL_WORD_134(local_work) &= 0xfffffeffu;
    }

    if (value < 1) {
      s16 next_value;

      next_value = BATTLE_LOCAL_HALF_88(local_work) - value;
      if ((u32)BATTLE_LOCAL_HALF_90(local_work) <= (u32)next_value) {
        BATTLE_LOCAL_HALF_88(local_work) =
            BATTLE_LOCAL_HALF_90(local_work);
        BATTLE_LOCAL_BYTE_120(local_work) |= 4u;
      } else {
        BATTLE_LOCAL_HALF_88(local_work) = next_value;
      }
    } else {
      s16 current;
      s16 next_value;

      current = BATTLE_LOCAL_HALF_88(local_work);
      next_value = current - value;
      if ((u32)current <= (u32)value) {
        if ((((value - current) < (s16)BATTLE_LOCAL_BYTE_8C(local_work)) &&
             ((BATTLE_LOCAL_WORD_128(local_work) & 1u) == 0u)) &&
            ((BATTLE_LOCAL_WORD_128(local_work) & 2u) == 0u)) {
          BATTLE_LOCAL_WORD_134(local_work) |= 4u;
        }
        BATTLE_LOCAL_HALF_88(local_work) = 0u;
      } else {
        BATTLE_LOCAL_HALF_88(local_work) = next_value;
      }
    }
  } else {
    enemy_work = &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu];
    if (BATTLE_ENEMY_HALF_F8(enemy_work) != 0xffffu) {
      if (((BATTLE_ENEMY_WORD_100(enemy_work) & 0x100u) != 0u) &&
          (BATTLE_ENEMY_HALF_F8(enemy_work) < 0x8000u)) {
        func_801de560(1u, 0u, 0u, 0x1eu, 0x801eb075u);
        BATTLE_UI_BYTE_83C3 = 1u;
        BATTLE_ENEMY_WORD_100(enemy_work) &= 0xfffffeffu;
        value = BATTLE_ENEMY_HALF_F8(enemy_work);
      }
      if (value < 1) {
        s16 next_value;

        next_value = BATTLE_ENEMY_HALF_F8(enemy_work) - value;
        if ((u32)BATTLE_ENEMY_HALF_A0(enemy_work) <= (u32)next_value) {
          BATTLE_ENEMY_HALF_F8(enemy_work) =
              BATTLE_ENEMY_HALF_A0(enemy_work);
          BATTLE_ENEMY_BYTE_FC(enemy_work) |= 4u;
        } else {
          BATTLE_ENEMY_HALF_F8(enemy_work) = next_value;
        }
      } else {
        s16 current;
        s16 next_value;

        current = BATTLE_ENEMY_HALF_F8(enemy_work);
        next_value = current - value;
        if ((u32)current <= (u32)value) {
          BATTLE_ENEMY_HALF_F8(enemy_work) = 0u;
        } else {
          BATTLE_ENEMY_HALF_F8(enemy_work) = next_value;
        }
      }
    }
  }

  if ((arg0 < 3u) &&
      (BATTLE_LOCAL_BYTE_13C(&BATTLE_LOCAL_WORK_ARRAY[arg0]) == 5u) &&
      (value != 0)) {
    if ((BATTLE_LOCAL_BYTE_82(&BATTLE_LOCAL_WORK_ARRAY[arg0]) ==
         'J') &&
        (func_800a3df8(arg0, arg1) == 0u)) {
      func_800a31e0(arg1, 0x40u);
    }
    if ((BATTLE_LOCAL_BYTE_82(&BATTLE_LOCAL_WORK_ARRAY[arg0]) ==
         'L') &&
        (func_800a3df8(arg0, arg1) == 0u)) {
      func_800a31e0(arg1, 8u);
    }
    if ((BATTLE_LOCAL_BYTE_82(&BATTLE_LOCAL_WORK_ARRAY[arg0]) ==
         'O') &&
        (func_800a3df8(arg0, arg1) == 0u)) {
      func_800a31e0(arg1, 0x20u);
    }
  }

  return (u32)(u16)value;
}
