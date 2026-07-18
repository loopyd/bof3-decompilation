#include "internal.h"

/* @behavior selects a target/result byte from the current enemy state and chosen
 * enemy kind, delegating special cases to several smaller picker helpers.
 * @source 0x801E2A88
 */
s8 func_801E2A88(u8 arg0) {
  volatile Battle03EnemyWork* battle_work;
  u16                         flags;
  u8                          kind_flags;
  s8                          value;

  battle_work = &BATTLE_ENEMY_WORK_ARRAY[arg0];
  if (BATTLE_GLOBAL_BYTE_6375 == 1u) {
    flags = BATTLE_ENEMY_FLAGS_80(battle_work);
    if ((flags & 1u) != 0u) {
      value = (s8)func_801E30B8((s8)arg0);
      if (value != -1) {
        return value;
      }
      return (s8)func_801E2E30();
    }
  } else {
    if (BATTLE_GLOBAL_BYTE_6375 != 4u) {
      return (s8)BATTLE_ENEMY_BYTE_05(battle_work);
    }

    flags = BATTLE_ENEMY_FLAGS_80(battle_work);
    kind_flags = BATTLE_KIND_BYTE_00(BATTLE_ENEMY_HALF_F6(battle_work));

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
        return (s8)func_801E2CA4();
      }
      if ((flags & 0x10u) == 0u) {
        return (s8)func_801E2E30();
      }
      return (s8)func_801E2D90();
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
      return (s8)func_801E2D4C((s8)arg0);
    }
  }

  if ((flags & 0x10u) != 0u) {
    return (s8)func_801E2D90();
  }
  return (s8)func_801E2E30();
}
