#include "internal.h"

/* does: computes one final signed damage value from source/target state, queued
 * base damage, status flags, and the current scratchpad damage modifiers.
 * @source: 0x801dc044 FUN_801dc044
 */
u32 func_801dc044(u8 arg0, u8 arg1, u16 arg2) {
  u16 flags;
  s32 value;
  u8  state_byte;

  *(volatile u16*)0x1f800000u = arg2;
  if (arg2 == 0xffffu) {
    if (arg0 < 3u) {
      *(volatile u16*)0x1f800000u =
          *(volatile u8*)(0x801d90ebu +
                          ((u32)BOF3_BATTLE_LOCAL_BYTE_82(
                               &BOF3_BATTLE_LOCAL_WORK_ARRAY[arg0]) *
                           0x18u));
    } else {
      *(volatile u16*)0x1f800000u = 0u;
    }
  }

  if (arg0 < 3u) {
    volatile Battle03LocalWork* source_work;

    source_work = &BOF3_BATTLE_LOCAL_WORK_ARRAY[arg0];
    if ((BOF3_BATTLE_LOCAL_WORD_128(source_work) & 0x40u) != 0u) {
      BOF3_BATTLE_GLOBAL_HALF_EC30C =
          BOF3_BATTLE_GLOBAL_HALF_EC30C +
          ((BOF3_BATTLE_GLOBAL_HALF_EC30C *
            BOF3_BATTLE_LOCAL_BYTE_138(source_work)) >>
           1);
      BOF3_BATTLE_LOCAL_BYTE_138(source_work) = 0u;
      BOF3_BATTLE_LOCAL_WORD_128(source_work) &= 0xffffffbfu;
    }
  } else {
    volatile Battle03EnemyWork* source_work;

    source_work = &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg0 - 3u) & 0xffu];
    if ((BOF3_BATTLE_ENEMY_WORD_104(source_work) & 0x40u) != 0u) {
      BOF3_BATTLE_GLOBAL_HALF_EC30C =
          (BOF3_BATTLE_GLOBAL_HALF_EC30C *
           BOF3_BATTLE_ENEMY_BYTE_114(source_work)) >>
          1;
      BOF3_BATTLE_ENEMY_BYTE_114(source_work) = 0u;
      BOF3_BATTLE_ENEMY_WORD_104(source_work) &= 0xffffffbfu;
    }
  }

  value = func_801dcad8(arg0, arg1, 0);

  if ((arg0 < 3u) && (arg1 >= 3u)) {
    u8 source_state;
    u8 target_state;

    source_state =
        BOF3_BATTLE_LOCAL_BYTE_82(&BOF3_BATTLE_LOCAL_WORK_ARRAY[arg0]);
    target_state = BOF3_BATTLE_ENEMY_BYTE_7D(
        &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]);
    if (((source_state == 0x13u) || (source_state == 0x17u) ||
         (source_state == 0x33u)) &&
        (target_state == 4u)) {
      value <<= 1;
    }
    if (((source_state == 0x16u) || (source_state == 0x35u) ||
         (source_state == 0x40u)) &&
        (target_state == 1u)) {
      value <<= 1;
    }
  }

  if (arg1 < 3u) {
    flags = BOF3_BATTLE_LOCAL_FLAGS_80(&BOF3_BATTLE_LOCAL_WORK_ARRAY[arg1]);
    state_byte =
        (u8)BOF3_BATTLE_LOCAL_WORD_134(&BOF3_BATTLE_LOCAL_WORK_ARRAY[arg1]);
  } else {
    flags = BOF3_BATTLE_ENEMY_FLAGS_82(
        &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]);
    state_byte = (u8)BOF3_BATTLE_ENEMY_WORD_100(
        &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]);
  }

  if ((flags & 0x64u) == 0u) {
    if (arg1 < 3u) {
      value = func_801dc73c((s16)value, arg0, arg1);
    } else {
      value = func_801dc894((s16)value, arg0, arg1);
    }
  } else {
    if ((flags & 0x40u) != 0u) {
      BOF3_BATTLE_GLOBAL_HALF_63DA |= 0x40u;
      if (arg1 < 3u) {
        func_800a36f0(
            BOF3_BATTLE_LOCAL_BYTE_05(&BOF3_BATTLE_LOCAL_WORK_ARRAY[arg1]),
            0x40u);
        BOF3_BATTLE_LOCAL_BYTE_121(&BOF3_BATTLE_LOCAL_WORK_ARRAY[arg1]) = 0u;
      } else {
        func_800a36f0(BOF3_BATTLE_ENEMY_BYTE_05(
                          &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]),
                      0x40u);
        BOF3_BATTLE_ENEMY_BYTE_FD(
            &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]) = 0u;
      }
      if ((u16)value == 0u) {
        value = 1u;
      }
    }
    if ((flags & 0x20u) != 0u) {
      BOF3_BATTLE_GLOBAL_HALF_63DA |= 0x20u;
      if (arg1 < 3u) {
        func_800a36f0(
            BOF3_BATTLE_LOCAL_BYTE_05(&BOF3_BATTLE_LOCAL_WORK_ARRAY[arg1]),
            0x20u);
        BOF3_BATTLE_LOCAL_BYTE_121(&BOF3_BATTLE_LOCAL_WORK_ARRAY[arg1]) = 0u;
      } else {
        func_800a36f0(BOF3_BATTLE_ENEMY_BYTE_05(
                          &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]),
                      0x20u);
        BOF3_BATTLE_ENEMY_BYTE_FD(
            &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu]) = 0u;
      }
      if ((u16)value == 0u) {
        value = 1u;
      }
    }
    if (((flags & 4u) != 0u) && ((u16)value == 0u)) {
      value = 1u;
    }
  }

  if ((state_byte & 2u) != 0u) {
    value =
        (value * BOF3_BATTLE_DAMAGE_SCALE_TABLE_0C7C[func_8017e3d4() & 7u]) /
        100;
  }

  if ((BOF3_BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) {
    value = (value << 1) + (((s32)func_801dcad8(arg0, arg1, 1) + 3) >> 2);
  }
  if ((arg1 < 3u) && (BOF3_BATTLE_GLOBAL_BYTE_44F58 == 2u)) {
    value -= value >> 2;
  }
  if ((flags & 0x80u) != 0u) {
    value -= value >> 2;
  }
  if (((flags & 0x60u) != 0u) && ((u16)value == 0u)) {
    value = 1u;
  }
  if (((BOF3_BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) && ((u16)value == 0u)) {
    value = 1u;
  }

  if ((state_byte & 0x10u) != 0u) {
    value = 0u;
  }
  if (value > 9999) {
    value = 9999;
  }
  if (value < -9999) {
    value = -9999;
  }

  return (u32)(s16)value;
}
