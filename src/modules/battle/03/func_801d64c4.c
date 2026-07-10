#include "internal.h"

/* does: reports whether one battler should be treated as blocked, taking the
 * global `0x10` suppression countdown into account before falling back to the
 * generic availability helper.
 * @source: 0x801d64c4 FUN_801d64c4
 */
u8 func_801d64c4(u32 arg0) {
  u32 flags;

  if (arg0 < 3u) {
    if (BATTLE_GLOBAL_BYTE_63CE != 0u) {
      flags = BATTLE_LOCAL_WORD_128(&BATTLE_LOCAL_WORK_ARRAY[arg0]);
      if ((flags & 0x10u) == 0u) {
        return 1u;
      }
    }
  } else {
    if (BATTLE_GLOBAL_BYTE_63CE != 0u) {
      flags = BATTLE_ENEMY_WORD_104(&BATTLE_ENEMY_WORK_ARRAY[arg0 - 3u]);
      if ((flags & 0x10u) == 0u) {
        return 1u;
      }
    }
  }

  return func_801db524((u8)arg0) != 0u;
}
