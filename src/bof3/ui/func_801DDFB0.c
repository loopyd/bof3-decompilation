#include "bof3/ui/shop00_internal.h"

/* @source 0x801DDFB0
 * @behavior draws a two-row shop value panel and submits its position: calls
 *           func_801DBF18 with the panel kind constants 7 and 4, emits the
 *           16x16 icon through func_801644D8 at (x + 7, y + 5) with the
 *           constants 16, 16 and 128, resolves one of the two icon palette
 *           shades through func_801502D0 (0x1C normally, 0x1B when the variant
 *           byte is zero) and draws the first row label at (x + 42, y + 7)
 *           with func_8014F800. It then draws the two value rows: each row
 *           emits its label through func_8014F800 at x + 6 and y + 25 or
 *           y + 41 with the shade 0 when the matching bit of the flags word is
 *           set and 7 otherwise, plus the texture 0x85, and its text through
 *           func_8014FF0C at (x + 94, same y) with the row string D_801D0E58 or
 *           D_801D0E5C. Finally it submits the cursor position through
 *           func_801647C4 at (x + 8, row * 16 + y + 28).
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The row pitch of the submitted cursor position is shifted into a named u16
 * local before the call: with the shift inlined the compiler emits
 * `addu a1,s5,a1`, while the named shift result makes it keep the original
 * `addu a1,a1,s5`. Two other shapes are load-bearing: the first palette shade
 * is a short-lived u32 local, which the compiler materializes straight into a0
 * with the pre-store in the branch delay slot, and the label x offset is an s16
 * local assigned before the first func_8014F800 call so the add fills the
 * func_801502D0 delay slot; an s32 spelling of that label local grows the
 * frame. The u8 `color` local supplies the original's single `andi 0xff`.
 */
void func_801DDFB0(s16 x, s16 y, u32 flags, u8 row, u8 variant) {
  s16 left;
  s16 right;
  s16 text_y;
  u16 row_offset;
  u8 color;
  u32 shade;
  u32 texture;

  func_801DBF18(x, y, 7, 4);
  func_801644D8(20, (u16)(x + 7), (u16)(y + 5), 16, 16, 128);
  shade = 0x1C;
  if (variant == 0) {
    shade = 0x1B;
  }
  texture = func_801502D0(shade);
  left = (s16)(x + 6);
  func_8014F800((s16)(x + 42), (s16)(y + 7), 0, 0xFF, texture);

  color = (flags & 1) ? 0 : 7;
  func_8014F800(left, (s16)(y + 25), color, 0xFF, func_801502D0(0x85));
  right = (s16)(x + 94);
  text_y = (s16)(y + 25);
  func_8014FF0C(right, text_y, color, D_801D0E58);

  color = (flags & 2) ? 0 : 7;
  func_8014F800(left, (s16)(y + 41), color, 0xFF, func_801502D0(0x85));
  text_y = (s16)(y + 41);
  func_8014FF0C(right, text_y, color, D_801D0E5C);

  row_offset = (u16)((u32)row << 4);
  func_801647C4((u16)(x + 8), (u16)(row_offset + y + 28), 0);
}
