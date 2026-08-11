#include "bof3/ui/shop00_internal.h"

/* @source 0x801DDFB0
 * @behavior draws a two-row shop value panel and submits its position.
 * @status partial
 * @match 93.22
 * @residual 468 current bytes versus 472 original; argument allocation and scheduling differ
 */
void func_801DDFB0(s16 x, s16 y, u32 flags, u8 row, u8 variant) {
  s16 left;
  s16 right;
  s16 text_y;
  u32 color;
  u32 texture;

  func_801DBF18(x, y, 7, 4);
  func_801644D8(20, (u16)(x + 7), (u16)(y + 5), 16, 16, 128);
  color = 0x1C;
  if (variant == 0) {
    color = 0x1B;
  }
  texture = func_801502D0(color);
  left = (s16)(x + 6);
  func_8014F800((s16)(x + 42), (s16)(y + 7), 0, 0xFF, texture);

  color = (flags & 1) ? 0 : 7;
  func_8014F800(left, (s16)(y + 25), color, 0xFF,
                func_801502D0(0x85));
  right = (s16)(x + 94);
  text_y = (s16)(y + 25);
  func_8014FF0C(right, text_y, color, D_801D0E58);

  color = (flags & 2) ? 0 : 7;
  func_8014F800(left, (s16)(y + 41), color, 0xFF,
                func_801502D0(0x85));
  text_y = (s16)(y + 41);
  func_8014FF0C(right, text_y, color, D_801D0E5C);

  func_801647C4((u16)(x + 8), (u16)(y + ((u32)row << 4) + 28), 0);
}
