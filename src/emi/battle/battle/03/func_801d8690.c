#include "internal.h"

/* @behavior draws one enemy status block, switching between panel-task text and a
 * fallback numeric label, then shades the lower border with a darkened palette
 * triple.
 * @source 0x801d8690 FUN_801d8690
 */
void func_801d8690(s32 arg0, s32 arg1, s32 arg2) {
  volatile Battle03EnemyWork* battle_work;
  u16                         color;
  s16                         red;
  s16                         green;
  s16                         blue;

  battle_work = &BATTLE_ENEMY_WORK_ARRAY[(arg2 - 3u) & 0xffu];

  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0, func_8017a620(0, 0, 0x3c0, 0),
                0);
  func_8014e5a0(1u, 0x0cu);

  color = *(volatile u16*)(0x80033a08u +
                           (((s32)BATTLE_GLOBAL_BYTE_4952 << 6) | 0x20));
  red = (color & 0x1fu) << 3;
  green = (color >> 2) & 0xf8u;
  blue = (color >> 7) & 0xf8u;

  func_801d9c80((s16)(arg0 + 4), (s16)(arg1 + 8), 8, 1);
  func_801d9ab4((s16)(arg0 + 4), (s16)(arg1 + 6), 2, 1);
  func_801d9ab4((s16)(arg0 + 4), (s16)(arg1 + 0x19), 3, 1);

  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0, func_8017a620(0, 1, 0x3c0, 0),
                0);
  func_8014e5a0(1u, 0x0cu);
  func_801d9c80((s16)arg0, (s16)(arg1 + 2), 8, 0);
  func_801d9dbc((s16)(arg0 + 6), (s16)(arg1 + 0x0b), 9, 0, 0);
  func_801d9ab4((s16)arg0, (s16)arg1, 2, 0);
  func_801d9ab4((s16)arg0, (s16)(arg1 + 0x13), 3, 0);

  if (BATTLE_ENEMY_BYTE_7F(battle_work) == 1u) {
    func_801d9e9c((s16)(arg0 + 6), (s16)(arg1 + 0x0a),
                  BATTLE_PANEL_TASK_BYTE_0B, BATTLE_PANEL_TASK_BYTE_0D, 1);
  } else {
    func_801d94d4((s16)(arg0 + 0x24), (u16)(arg1 + 0x0c), 0, -1);
  }

  func_8014fc90((s16)(arg0 + 4), (s16)(arg1 + 3), 0, 8,
                (void*)((volatile u8*)battle_work + 0x74u));

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
