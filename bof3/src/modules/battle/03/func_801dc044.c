#include "internal.h"

typedef struct Battle03DamageScaleTable {
  u8 values[8];
} __attribute__((packed)) Battle03DamageScaleTable;

/* does: computes one final signed damage value from source/target state, queued
 * base damage, status flags, and the current scratchpad damage modifiers.
 * @source: 0x801dc044 FUN_801dc044
 */
u32 func_801dc044(u8 arg0, u8 arg1, u16 arg2) {
  Battle03DamageScaleTable damage_scale_table;
  u16 flags;
  s32 value;
  u8  state_byte;

  damage_scale_table =
      *(const Battle03DamageScaleTable*)BATTLE_DAMAGE_SCALE_TABLE_0C7C;
  if (arg2 == 0xffffu) {
    if (arg0 < 3u) {
      BATTLE_SCRATCH_HALF_000 =
          BATTLE_PANEL_SLOT_MASK(BATTLE_LOCAL_BYTE_82(
              &BATTLE_LOCAL_WORK_ARRAY[arg0]));
    } else {
      BATTLE_SCRATCH_HALF_000 = 0u;
    }
  } else {
    BATTLE_SCRATCH_HALF_000 = arg2;
  }

  if (arg0 < 3u) {
    volatile Battle03LocalWork* source_work;

    source_work = &BATTLE_LOCAL_WORK_ARRAY[arg0];
    if ((BATTLE_LOCAL_WORD_128(source_work) & 0x40u) != 0u) {
      BATTLE_GLOBAL_HALF_EC30C =
          BATTLE_GLOBAL_HALF_EC30C +
          ((BATTLE_GLOBAL_HALF_EC30C *
            BATTLE_LOCAL_BYTE_138(source_work)) >>
           1);
      BATTLE_LOCAL_BYTE_138(source_work) = 0u;
      BATTLE_LOCAL_WORD_128(source_work) &= 0xffffffbfu;
    }
  } else {
    volatile Battle03EnemyWork* source_work;

    source_work = &BATTLE_ENEMY_WORK_ARRAY[(arg0 - 3u) & 0xffu];
    if ((BATTLE_ENEMY_WORD_104(source_work) & 0x40u) != 0u) {
      BATTLE_GLOBAL_HALF_EC30C =
          (BATTLE_GLOBAL_HALF_EC30C *
           BATTLE_ENEMY_BYTE_114(source_work)) >>
          1;
      BATTLE_ENEMY_BYTE_114(source_work) = 0u;
      BATTLE_ENEMY_WORD_104(source_work) &= 0xffffffbfu;
    }
  }

  value = func_801dcad8(arg0, arg1, 0);

  if ((arg0 < 3u) && (arg1 >= 3u)) {
    u8 source_state;

    source_state =
        BATTLE_LOCAL_BYTE_82(&BATTLE_LOCAL_WORK_ARRAY[arg0]);
    if (((source_state == 0x13u) || (source_state == 0x17u) ||
         (source_state == 0x33u)) &&
        (BATTLE_ENEMY_BYTE_7D(
             &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]) == 4u)) {
      value = ((u32)value << 16) >> 15;
    }
    if (((source_state == 0x16u) || (source_state == 0x35u) ||
         (source_state == 0x40u)) &&
        (BATTLE_ENEMY_BYTE_7D(
             &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]) == 1u)) {
      value = ((u32)value << 16) >> 15;
    }
  }

  if (arg1 < 3u) {
    flags = BATTLE_LOCAL_FLAGS_80(&BATTLE_LOCAL_WORK_ARRAY[arg1]);
    state_byte =
        BATTLE_LOCAL_BYTE_134(&BATTLE_LOCAL_WORK_ARRAY[arg1]);
  } else {
    flags = BATTLE_ENEMY_FLAGS_82(
        &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]);
    state_byte = BATTLE_ENEMY_BYTE_100(
        &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]);
  }

  if ((flags & 0x64u) != 0u) {
    if ((flags & 0x40u) != 0u) {
      BATTLE_GLOBAL_HALF_63DA |= 0x40u;
      if (arg1 < 3u) {
        func_800a36f0(
            BATTLE_LOCAL_BYTE_05(&BATTLE_LOCAL_WORK_ARRAY[arg1]),
            0x40u);
        BATTLE_LOCAL_BYTE_121(&BATTLE_LOCAL_WORK_ARRAY[arg1]) = 0u;
      } else {
        func_800a36f0(BATTLE_ENEMY_BYTE_05(
                          &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]),
                      0x40u);
        BATTLE_ENEMY_BYTE_FD(
            &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]) = 0u;
      }
      if ((s16)value == 0) {
        value = 1u;
      }
    }
    if ((flags & 0x20u) != 0u) {
      BATTLE_GLOBAL_HALF_63DA |= 0x20u;
      if (arg1 < 3u) {
        func_800a36f0(
            BATTLE_LOCAL_BYTE_05(&BATTLE_LOCAL_WORK_ARRAY[arg1]),
            0x20u);
        BATTLE_LOCAL_BYTE_121(&BATTLE_LOCAL_WORK_ARRAY[arg1]) = 0u;
      } else {
        func_800a36f0(BATTLE_ENEMY_BYTE_05(
                          &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]),
                      0x20u);
        BATTLE_ENEMY_BYTE_FD(
            &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]) = 0u;
      }
      if ((s16)value == 0) {
        value = 1u;
      }
    }
    if (((flags & 4u) != 0u) && ((s16)value == 0)) {
      value = 1u;
    }
  } else {
    if (arg1 < 3u) {
      value = func_801dc73c((s16)value, arg0, arg1);
    } else {
      value = func_801dc894((s16)value, arg0, arg1);
    }
  }

  if ((state_byte & 2u) != 0u) {
    value =
        ((s16)value *
         damage_scale_table.values[((s32 (*)(void))func_8017e3d4)() % 8]) /
        100;
  }

  if ((BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) {
    s16 bonus;

    bonus = (s16)func_801dcad8(arg0, arg1, 1);
    if (bonus < 0) {
      bonus += 3;
    }
    value = ((s16)value << 1) + (bonus >> 2);
  }
  if ((arg1 < 3u) && (BATTLE_GLOBAL_BYTE_44F58 == 2u)) {
    value -= (s16)value >> 2;
  }
  if ((flags & 0x80u) != 0u) {
    value -= (s16)value >> 2;
  }
  if (((flags & 0x60u) != 0u) && ((s16)value == 0)) {
    value = 1u;
  }
  if (((BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) && ((s16)value == 0)) {
    value = 1u;
  }

  if (arg1 < 3u) {
    if ((BATTLE_LOCAL_WORD_134(&BATTLE_LOCAL_WORK_ARRAY[arg1]) &
         0x10000u) != 0u) {
      value = 0u;
    }
  } else {
    if ((BATTLE_ENEMY_WORD_100(
             &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]) &
         0x10000u) != 0u) {
      value = 0u;
    }
  }
  if ((s16)value > 9999) {
    value = 9999;
  }
  if ((s16)value < -9999) {
    value = -9999;
  }

  return (u32)(s16)value;
}
