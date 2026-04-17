#include "internal.h"

/* does: derives one enemy target mode from the packed mode byte and current
 * enemy flags, runs one of two target-selection paths, optionally refreshes the
 * current enemy callback, then stores the final mode byte back to enemy work.
 * @source: 0x801e25e0 FUN_801e25e0
 */
void func_801e25e0(u8 arg0) {
  volatile Battle03EnemyWork* battle_work;
  u8                          mode;
  u8                          packed;

  battle_work = &BOF3_BATTLE_ENEMY_WORK_ARRAY[arg0];

  packed = BOF3_BATTLE_TARGET_MODE_PACK(BOF3_BATTLE_ENEMY_BYTE_7E(battle_work));
  mode = (packed >> ((func_8017e3d4() & 3u) << 1)) & 3u;
  BOF3_BATTLE_GLOBAL_BYTE_6375 = mode;

  if (((BOF3_BATTLE_GLOBAL_HALF_62E8 & 0x4000u) != 0u) &&
      (BOF3_BATTLE_ENEMY_HALF_AA(battle_work) < 2u)) {
    BOF3_BATTLE_GLOBAL_BYTE_6375 = 0u;
  }

  if (BOF3_BATTLE_GLOBAL_BYTE_6375 == 1u) {
    if ((BOF3_BATTLE_ENEMY_FLAGS_80(battle_work) & 0x80u) != 0u) {
      BOF3_BATTLE_GLOBAL_BYTE_6375 = 2u;
      BOF3_BATTLE_ENEMY_WORD_100(battle_work) |= 2u;
    }
  } else if (BOF3_BATTLE_GLOBAL_BYTE_6375 == 2u) {
    if ((BOF3_BATTLE_ENEMY_FLAGS_80(battle_work) & 0x40u) != 0u) {
      BOF3_BATTLE_GLOBAL_BYTE_6375 = 3u;
    }
  } else if (BOF3_BATTLE_GLOBAL_BYTE_6375 == 3u) {
    if ((BOF3_BATTLE_ENEMY_FLAGS_80(battle_work) & 0x20u) != 0u) {
      BOF3_BATTLE_GLOBAL_BYTE_6375 = 4u;
      BOF3_BATTLE_ENEMY_HALF_F6(battle_work) = func_8017e3d4() & 7u;
    }
  } else if (BOF3_BATTLE_GLOBAL_BYTE_6375 != 0u) {
    BOF3_BATTLE_GLOBAL_BYTE_6375 = 1u;
  }

  if (((BOF3_BATTLE_ENEMY_FLAGS_82(battle_work) & 0x20u) == 0u) &&
      ((BOF3_BATTLE_ENEMY_WORD_104(battle_work) & 0x4000u) == 0u)) {
    if (func_800a9304(arg0 + 3u) != 0u) {
      BOF3_BATTLE_GLOBAL_BYTE_6384 = func_801e2a88(arg0);
    }
  } else {
    BOF3_BATTLE_GLOBAL_BYTE_6375 = 1u;
    if (func_800a9304(arg0 + 3u) != 0u) {
      func_801e2948((s8)arg0);
    }
  }

  if (BOF3_BATTLE_GLOBAL_BYTE_62EA != 0u) {
    BOF3_BATTLE_ENEMY_SCRATCH_PTR = battle_work;
    BOF3_BATTLE_CURRENT_ENEMY_PTR = battle_work;
    battle_work->unk_e4(0);
  }

  if (BOF3_BATTLE_GLOBAL_BYTE_6384 == 0xffu) {
    BOF3_BATTLE_GLOBAL_BYTE_6375 = 0u;
    BOF3_BATTLE_ENEMY_BYTE_F5(battle_work) = 0u;
  } else {
    BOF3_BATTLE_ENEMY_BYTE_F5(battle_work) = BOF3_BATTLE_GLOBAL_BYTE_6375;
  }
}
