#include "bof3/bof3.h"

/* Panel backing drawer of this overlay: takes the panel origin and the cell
 * counts 7 and 4, which this body passes as its cell grid.
 * @source 0x801D6470 */
extern void func_801D6470(s16 arg0, s16 arg1, s32 arg2, s32 arg3);

/* Main-exe sprite-record helper: emits the 16 by 16 icon whose id is the first
 * argument at the position given by the next two, with the size pair and the
 * attribute word 128 following.
 * @source 0x801644D8 */
extern void func_801644D8(u32 arg0, u32 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u32 arg5);

/* Main-exe text handle query: returns the handle that the byte index selects,
 * which this body hands on as the text argument of func_8014F800.
 * @source 0x801502D0 */
extern u32 func_801502D0(u32 arg0);

/* Main-exe caption drawer: the two signed coordinates, the alignment argument,
 * the 0xFF mask and the text handle.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Main-exe line-text drawer: the two signed coordinates, the shade the flags
 * select and the string it draws.
 * @source 0x8014FF0C */
extern void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);

/* Main-exe panel cursor helper: submits the marker position built from this
 * body's panel origin and the selected row.
 * @source 0x801647C4 */
extern void func_801647C4(u16 arg0, u16 arg1, s32 arg2);

/* The two row-number strings of this overlay's leading data blob: the single
 * characters "1" at 0x801D0D94 and "2" at 0x801D0D98, one per row of this
 * panel, drawn at the label column x + 94. */
extern const char D_801D0D94[]; /* @source 0x801D0D94 @kind data */
extern const char D_801D0D98[]; /* @source 0x801D0D98 @kind data */

/* @source 0x801D8508
 * @behavior Two-row numbered panel of the BATE overlay, the copy of the shop
 *           value panel in emi/etc/shop/00 at 0x801DDFB0: takes the panel
 *           origin, a flags word, the selected row and a variant byte. It draws
 *           the 7 by 4 cell backing through this overlay's own func_801D6470
 *           with the counts 7 and 4, emits the 16 by 16 icon 0x14 through
 *           func_801644D8 at (x + 7, y + 5) with the size pair 16/16 and the
 *           attribute 128, and the caption through func_8014F800 at
 *           (x + 42, y + 7) with the shade 0 and the 0xFF mask. That caption
 *           handle is func_801502D0(0x1C), or func_801502D0(0x1B) when the
 *           variant byte is zero. It then draws the two rows at y + 25 and
 *           y + 41: each row emits the label func_801502D0(0x85) through
 *           func_8014F800 at x + 6 with the shade 0 when the matching bit of the
 *           flags word is set and 7 otherwise, plus the 0xFF mask, and its value
 *           through func_8014FF0C at x + 94 with the same shade, which is the
 *           string "1" for the first row and "2" for the second. Finally it
 *           submits the row marker through func_801647C4 at (x + 8,
 *           row * 16 + y + 28). Nothing is returned, and the 0x40-byte frame
 *           keeps eight saved registers and the return address across the
 *           calls while reading the variant byte from the caller's stack
 *           argument word. No jal and no table word of this payload names the
 *           span, so it is an overlay entry called from outside the payload and
 *           its arguments have the roles their uses give them. A cmp of this
 *           0x1D8-byte span against the emi/etc/shisu/00 copy at 0x801D94B0,
 *           itself exact with the shop copy, left three words different: the
 *           jal to this payload's own func_801D6470 where that copy calls
 *           0x801D7418, and the two literal low halves at +0x120 and +0x178,
 *           each 0x1C above that copy's row strings 0x801D0D78 and 0x801D0D7C,
 *           so this is one body with this overlay's bindings.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTwoRowNumberedPanelBate00_801D8508(s16 x, s16 y, u32 flags, u8 row,
                                            u8 variant) {
  s16 left;
  s16 right;
  s16 text_y;
  u16 row_offset;
  u8 color;
  u32 shade;
  u32 texture;

  func_801D6470(x, y, 7, 4);
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
  func_8014FF0C(right, text_y, color, D_801D0D94);

  color = (flags & 2) ? 0 : 7;
  func_8014F800(left, (s16)(y + 41), color, 0xFF, func_801502D0(0x85));
  text_y = (s16)(y + 41);
  func_8014FF0C(right, text_y, color, D_801D0D98);

  row_offset = (u16)((u32)row << 4);
  func_801647C4((u16)(x + 8), (u16)(row_offset + y + 28), 0);
}
