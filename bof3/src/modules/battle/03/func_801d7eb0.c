#include "internal.h"

/* does: prepares one UI/frame primitive group using the current palette-derived
 * rgb triple, then emits the surrounding lines/boxes for that anchor.
 * @source: 0x801d7eb0 FUN_801d7eb0
 */
void func_801d7eb0(s32 arg0, s32 arg1) {
  s16 width;
  s16 x0;
  s16 y0;
  s16 x1;
  s16 y1;
  u16 color;
  u8  r;
  u8  g;
  u8  b;
  s32 clut_index;

  width = func_8017a620(0, 0, 0x3c0, 0);
  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0, width, 0);
  func_8014e5a0(1u, 0x0cu);

  x0 = (s16)arg0;
  y0 = (s16)(arg1 + 2);

  clut_index = (s32)BATTLE_GLOBAL_BYTE_4952;
  color = *(volatile u16*)(0x80033a08u + ((clut_index << 6) | 0x20));
  r = (color & 0x1fu) << 3;
  g = (color >> 2) & 0xf8u;
  b = (color >> 7) & 0xf8u;

  func_801d9c80(x0, y0, 3, 1);
  func_801d9ab4(x0, (s16)arg1, 8, 1);
  func_801d9ab4(x0, (s16)(arg1 + 0x11), 9, 1);

  x0 = (s16)(arg0 + 2);
  x1 = (s16)(arg0 + 0x115);
  func_801da5a8(x0, y0, x1, y0, r, g, b);

  y1 = (s16)(arg1 + 0x10);
  func_801da5a8(x0, (s16)(arg1 + 3), x0, y1, r, g, b);
  func_801da4b4(x1, y0, x1, y1, r, g, b);
  func_801da4b4(x0, y1, x1, y1, r, g, b);
}
