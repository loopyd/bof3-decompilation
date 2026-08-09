#include "bof3/battle/battle03_internal.h"

/* @behavior reports whether all active local battlers satisfy the strict ready
 * predicate used by the later queued branch.
 * @source 0x801E0E0C
 * @status partial
 * @match 49.28
 * @residual non-exact live audit: 34/64 instructions; 256 original bytes versus 276 current.
 */
u8 func_801E0E0C(void) {
  u8 index;

  index = 0u;
  while (index < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if ((battle_work->flags_00 & 1u) != 0u) {
      if ((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x4004u) == 0u) {
        return 0u;
      }
      if ((BATTLE_LOCAL_WORD_124(battle_work) & 4u) != 0u) {
        return 0u;
      }
      if ((BATTLE_LOCAL_WORD_128(battle_work) & 2u) != 0u) {
        return 0u;
      }
      if ((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x4000u) != 0u) {
        if (BATTLE_LOCAL_BYTE_86(battle_work) == 0x18u) {
          return 0u;
        }
        if (BATTLE_LOCAL_BYTE_87(battle_work) == 0x18u) {
          return 0u;
        }
        if (BATTLE_LOCAL_BYTE_85(battle_work) == 'C') {
          return 0u;
        }
      }
    }
    index += 1u;
  }
  return 1u;
}
