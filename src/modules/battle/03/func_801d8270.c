#include "internal.h"

/* @behavior draws one compact frame using the shared border helpers and the current
 * palette-derived rgb triple.
 * @source 0x801d8270 FUN_801d8270
 */
void func_801d8270(s32 arg0, s32 arg1) {
  u16 color;
  u8  red;
  u8  green;
  u8  blue;

  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0, func_8017a620(0, 0, 0x3c0, 0),
                0);
  func_8014e5a0(1u, 0x0cu);

  color = *(volatile u16*)(0x80033a08u +
                           (((s32)BATTLE_GLOBAL_BYTE_4952 << 6) | 0x20));
  red = (color & 0x1fu) << 3;
  green = (color >> 2) & 0xf8u;
  blue = (color >> 7) & 0xf8u;

  func_801d9c80((s16)arg0, (s16)(arg1 + 2), 10, 1);
  func_801d9ab4((s16)arg0, (s16)arg1, 6, 1);
  func_801d9ab4((s16)arg0, (s16)(arg1 + 0x11), 7, 1);
  func_801da5a8((s16)(arg0 + 2), (s16)(arg1 + 2), (s16)(arg0 + 0x65),
                (s16)(arg1 + 2), red, green, blue);
  func_801da5a8((s16)(arg0 + 2), (s16)(arg1 + 3), (s16)(arg0 + 2),
                (s16)(arg1 + 0x10), red, green, blue);
  func_801da4b4((s16)(arg0 + 0x65), (s16)(arg1 + 2), (s16)(arg0 + 0x65),
                (s16)(arg1 + 0x10), red, green, blue);
  func_801da4b4((s16)(arg0 + 2), (s16)(arg1 + 0x10), (s16)(arg0 + 0x65),
                (s16)(arg1 + 0x10), red, green, blue);
}
