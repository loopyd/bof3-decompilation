#include "internal.h"

/* @behavior draws one indexed panel frame and icon from the packed table at
 * `0x801eaee8`, then submits the matching icon from `0x801eaeb0`.
 * @source 0x801d8450 FUN_801d8450
 */
void func_801d8450(u32 arg0) {
  s16 x;
  s16 y;
  u16 color;
  u8  red;
  u8  green;
  u8  blue;
  u32 table_index;

  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0, func_8017a620(0, 0, 0x3c0, 0),
                0);
  func_8014e5a0(1u, 0x0cu);

  table_index = (arg0 & 0xffu) * 4u;
  x = BATTLE_PANEL_FRAME_TABLE_AEE8[table_index + 0];
  y = BATTLE_PANEL_FRAME_TABLE_AEE8[table_index + 1];

  color = *(volatile u16*)(0x80033a08u +
                           (((s32)BATTLE_GLOBAL_BYTE_4952 << 6) | 0x20));
  red = (color & 0x1fu) << 3;
  green = (color >> 2) & 0xf8u;
  blue = (color >> 7) & 0xf8u;

  func_801d9c80(x, (s16)(y + 2), 5, 1);
  func_801d9ab4(x, y, 10, 1);
  func_801d9ab4(x, (s16)(y + 0x11), 0xb, 1);
  func_801da5a8((s16)(x + 2), (s16)(y + 2), (s16)(x + 0x25), (s16)(y + 2), red,
                green, blue);
  func_801da5a8((s16)(x + 2), (s16)(y + 3), (s16)(x + 2), (s16)(y + 0x10), red,
                green, blue);
  func_801da4b4((s16)(x + 0x25), (s16)(y + 2), (s16)(x + 0x25), (s16)(y + 0x10),
                red, green, blue);
  func_801da4b4((s16)(x + 2), (s16)(y + 0x10), (s16)(x + 0x25), (s16)(y + 0x10),
                red, green, blue);
  func_8014f800((s16)(x + 8), (s16)(y + 3), 0, 8,
                BATTLE_PANEL_ICON_TABLE_AEB0[arg0 & 0xffu]);
}
