#include "internal.h"

/* does: advances one local `0x2` movement path, either promoting it directly to
 * a presentation state or converting the remaining distance into a queued
 * countdown substate.
 * @source: 0x801d62d8 FUN_801d62d8
 */
u8 func_801d62d8(void) {
  u8 index;

  index = 0u;
  do {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if (func_801d64c4(index) == 0u) {
      if (((BATTLE_LOCAL_WORD_128(battle_work) & 2u) != 0u) &&
          ((BATTLE_LOCAL_WORD_128(battle_work) & 0x20u) == 0u)) {
        if ((u32)BATTLE_LOCAL_HALF_8A(battle_work) <
            (u32)((BATTLE_GLOBAL_HALF_63B8 + 1u) >> 1)) {
          battle_work->unk_01 = 6u;
          battle_work->unk_02 = 4u;
          battle_work->unk_03 = 4u;
          func_801de190(index);
        } else {
          BATTLE_LOCAL_HALF_1C(battle_work) = 0u;
          BATTLE_LOCAL_HALF_1E(battle_work) = 0u;
          BATTLE_LOCAL_HALF_8A(battle_work) -=
              (BATTLE_GLOBAL_HALF_63B8 + 1u) >> 1;
          BATTLE_LOCAL_HALF_1E(battle_work) =
              (BATTLE_GLOBAL_HALF_63B8 + 1u) >> 1;
          if (BATTLE_LOCAL_HALF_1E(battle_work) != 0u) {
            battle_work->unk_02 = 5u;
            battle_work->unk_01 = 6u;
            battle_work->unk_04 = 0u;
            battle_work->unk_03 = 0u;
            battle_work->unk_20 = 2u;
            func_801de190(index);
          }
        }
      } else {
        BATTLE_LOCAL_WORD_128(battle_work) &= 0xffffffdful;
      }
    }
    index += 1u;
  } while (index < 3u);

  return BATTLE_GLOBAL_HALF_63C2 != 0u;
}
