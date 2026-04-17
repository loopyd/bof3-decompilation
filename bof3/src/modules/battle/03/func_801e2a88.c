#include "internal.h"

/* does: selects a target/result byte from the current enemy state and chosen
 * enemy kind, delegating special cases to several smaller picker helpers.
 * @source: 0x801e2a88 FUN_801e2a88
 */
s8 func_801e2a88(u8 arg0) {
  volatile Battle03EnemyWork* battle_work;
  u16                         flags;
  u8                          kind_flags;
  s8                          value;

  battle_work = &BOF3_BATTLE_ENEMY_WORK_ARRAY[arg0];
  if (BOF3_BATTLE_GLOBAL_BYTE_6375 == 1u) {
    flags = BOF3_BATTLE_ENEMY_FLAGS_80(battle_work);
    if ((flags & 1u) != 0u) {
      value = (s8)func_801e30b8((s8)arg0);
      if (value != -1) {
        return value;
      }
      return (s8)func_801e2e30();
    }
  } else {
    if (BOF3_BATTLE_GLOBAL_BYTE_6375 != 4u) {
      return (s8)BOF3_BATTLE_ENEMY_BYTE_05(battle_work);
    }

    flags = BOF3_BATTLE_ENEMY_FLAGS_80(battle_work);
    kind_flags =
        BOF3_BATTLE_KIND_BYTE_00(BOF3_BATTLE_ENEMY_HALF_F6(battle_work));

    if ((flags & 1u) == 0u) {
      if ((kind_flags & 0x10u) != 0u) {
        if (((kind_flags & 0x80u) != 0u) && ((kind_flags & 0x40u) == 0u)) {
          return -0x40;
        }
        if ((kind_flags & 0x20u) != 0u) {
          return -0x80;
        }
        return 0x40;
      }
      if ((kind_flags & 0x40u) == 0u) {
        return (s8)(arg0 + 3u);
      }
      if ((kind_flags & 0x20u) == 0u) {
        return (s8)func_801e2ca4();
      }
      if ((flags & 0x10u) == 0u) {
        return (s8)func_801e2e30();
      }
      return (s8)func_801e2d90();
    }

    if ((kind_flags & 0x10u) != 0u) {
      if (((kind_flags & 0x80u) != 0u) && ((kind_flags & 0x40u) == 0u)) {
        return -0x40;
      }
      if ((kind_flags & 0x20u) != 0u) {
        return 0x40;
      }
      return -0x80;
    }
    if ((kind_flags & 0x40u) == 0u) {
      return (s8)(arg0 + 3u);
    }
    if ((kind_flags & 0x20u) != 0u) {
      return (s8)func_801e2d4c((s8)arg0);
    }
  }

  if ((flags & 0x10u) != 0u) {
    return (s8)func_801e2d90();
  }
  return (s8)func_801e2e30();
}
