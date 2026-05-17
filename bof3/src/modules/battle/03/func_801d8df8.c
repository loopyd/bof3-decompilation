#include "internal.h"

/* does: draws one local battler detail block with icon, numeric readouts, and a
 * two-stage shaded border based on the battler's local flags and values.
 * @source: 0x801d8df8 FUN_801d8df8
 */
void func_801d8df8(s32 arg0, s32 arg1, u32 arg2) {
  volatile Battle03LocalWork* battle_work;
  u16                         color;
  s16                         red;
  s16                         green;
  s16                         blue;
  u32                         mode;

  battle_work = &BATTLE_LOCAL_WORK_ARRAY[arg2 & 0xffu];

  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0,
                func_8017a620(0, 0, 0x3c0, 0), 0);
  func_8014e5a0(1u, 0x0cu);

  color = *(volatile u16*)(0x80033a08u +
                           (((s32)BATTLE_GLOBAL_BYTE_4952 << 6) | 0x20));
  red = (color & 0x1fu) << 3;
  green = (color >> 2) & 0xf8u;
  blue = (color >> 7) & 0xf8u;

  func_801d9c80((s16)(arg0 + 4), (s16)(arg1 + 8), 8, 1);
  func_801d9ab4((s16)(arg0 + 4), (s16)(arg1 + 6), 2, 1);
  func_801d9ab4((s16)(arg0 + 4), (s16)(arg1 + 0x19), 3, 1);

  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0,
                func_8017a620(0, 1, 0x3c0, 0), 0);
  func_8014e5a0(1u, 0x0cu);
  func_801d9c80((s16)arg0, (s16)(arg1 + 2), 8, 0);
  func_801d9ab4((s16)arg0, (s16)arg1, 2, 0);
  func_801d9ab4((s16)arg0, (s16)(arg1 + 0x13), 3, 0);

  mode = (BATTLE_LOCAL_FLAGS_80(battle_work) & 0x4000u) != 0u
             ? 2u
             : ((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x0bfcu) != 0u);
  func_8014fc90((s16)(arg0 + 4), (s16)(arg1 + 2), mode, 5,
                (void*)((volatile u8*)battle_work + 0x44u));

  func_801da078((s16)(arg0 + 2), (s16)(arg1 + 10), 0);
  func_801d94d4((s16)(arg0 + 0x14), (u16)(arg1 + 0x0c),
                (BATTLE_LOCAL_FLAGS_80(battle_work) & 0x4000u) != 0u
                    ? 2u
                    : ((BATTLE_LOCAL_FLAGS_80(battle_work) >> 11) & 4u),
                BATTLE_LOCAL_HALF_88(battle_work));

  func_8017e3f4((void*)BATTLE_UI_CHAR_BUFFER, (const void*)0x801d0c6cu,
                (BATTLE_LOCAL_FLAGS_80(battle_work) >> 13) & 2u);
  func_80150098((s16)(arg0 + 0x28), (s16)(arg1 + 10),
                (BATTLE_LOCAL_FLAGS_80(battle_work) >> 13) & 2u,
                (void*)BATTLE_UI_CHAR_BUFFER);
  func_801d94d4((s16)(arg0 + 0x30), (u16)(arg1 + 0x0c),
                (BATTLE_LOCAL_FLAGS_80(battle_work) >> 13) & 2u,
                BATTLE_LOCAL_HALF_90(battle_work));

  func_801da5a8((s16)(arg0 + 2), (s16)(arg1 + 2), (s16)(arg0 + 0x49),
                (s16)(arg1 + 2), (u8)red, (u8)green, (u8)blue);
  func_801da5a8((s16)(arg0 + 2), (s16)(arg1 + 3), (s16)(arg0 + 2),
                (s16)(arg1 + 0x12), (u8)red, (u8)green, (u8)blue);

  red -= 0x10;
  green -= 0x10;
  blue -= 0x10;
  if (red < 0) {
    red = 0;
  } else if (red > 0xff) {
    red = 0xff;
  }
  if (green < 0) {
    green = 0;
  } else if (green > 0xff) {
    green = 0xff;
  }
  if (blue < 0) {
    blue = 0;
  } else if (blue > 0xff) {
    blue = 0xff;
  }

  func_801da408((s16)(arg0 + 0x49), (s16)(arg1 + 2), (s16)(arg0 + 0x49),
                (s16)(arg1 + 0x12), (u8)red, (u8)green, (u8)blue);
  func_801da408((s16)(arg0 + 2), (s16)(arg1 + 0x12), (s16)(arg0 + 0x49),
                (s16)(arg1 + 0x12), (u8)red, (u8)green, (u8)blue);
}
