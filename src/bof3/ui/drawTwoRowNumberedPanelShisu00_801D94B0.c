#include "bof3/bof3.h"
#include "gpu/prim.h"

/* The overlay's panel-backing drawer, still an `asm` row of this target's Splat
 * layout, so only its call is bound here. It takes the panel origin and the cell
 * counts 7 and 4 (it reads each count as a byte, multiplies it by 8 and adds the
 * 0x2D and 0x18 origin biases), so this body asks for a 7 by 4 cell grid.
 * @source 0x801D7418 */
extern void func_801D7418(s16 arg0, s16 arg1, s32 arg2, s32 arg3);

/* Main-executable sprite-record helper: emits the 16 by 16 icon whose id is the
 * first argument at the position given by the next two, with the size pair and
 * the attribute word 128 following.
 * @source 0x801644D8 */
extern void func_801644D8(u32 arg0, u32 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u32 arg5);

/* Main-executable handle query: returns the text handle that the byte index
 * selects, which this body hands on as the text argument of func_8014F800.
 * @source 0x801502D0 */
extern u32 func_801502D0(u32 arg0);

/* Main-executable line-text drawer used at every caption and label site of this
 * body: the first two arguments are the row position, the third is the shade
 * the flags select and the fourth the 0xFF mask.
 * @source 0x8014FF0C */
extern void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);

/* Main-executable panel cursor helper: submits the marker position built from
 * this body's panel origin and the selected row.
 * @source 0x801647C4 */
extern void func_801647C4(u16 arg0, u16 arg1, s32 arg2);

/* The two row-value strings of the overlay's initialized blob: the single
 * characters "1" at 0x801D0D78 and "2" at 0x801D0D7C, one per row of this
 * panel, drawn at the label column x + 94.
 */
extern const char D_801D0D78[]; /* @source 0x801D0D78 @kind data */
extern const char D_801D0D7C[]; /* @source 0x801D0D7C @kind data */

/* @source 0x801D94B0
 * @behavior Two-row numbered panel of the SHISU overlay, the copy of the shop
 *           value panel in emi/etc/shop/00 at 0x801DDFB0: takes the panel
 *           origin, a flags word, the selected row and a variant byte. It draws
 *           the 7 by 4 cell backing through the overlay's own func_801D7418
 *           with the counts 7 and 4, the 16 by 16 icon 0x14 through
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
 *           row * 16 + y + 28). No jal and no table word of this payload names
 *           this body, so it is an overlay entry called from outside the
 *           payload. Its 0x40-byte frame keeps eight saved registers and $ra,
 *           reads the variant byte from the caller's stack argument word, and
 *           the epilogue leaves the cursor helper's result in $v0. The body is
 *           the shop copy's byte for byte except three words: the jal to this
 *           payload's own 0x801D7418, where the shop copy calls its
 *           0x801DBF18 at the same span-relative distance, and the two
 *           low-halfword adds that carry this payload's string table base,
 *           0xE0 below the shop copy's in both.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTwoRowNumberedPanelShisu00_801D94B0(s16 x, s16 y, u32 flags, u8 row,
                                             u8 variant) {
  s16 left;
  s16 right;
  s16 text_y;
  u16 row_offset;
  u8 color;
  u32 shade;
  u32 texture;

  func_801D7418(x, y, 7, 4);
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
  func_8014FF0C(right, text_y, color, D_801D0D78);

  color = (flags & 2) ? 0 : 7;
  func_8014F800(left, (s16)(y + 41), color, 0xFF, func_801502D0(0x85));
  text_y = (s16)(y + 41);
  func_8014FF0C(right, text_y, color, D_801D0D7C);

  row_offset = (u16)((u32)row << 4);
  func_801647C4((u16)(x + 8), (u16)(row_offset + y + 28), 0);
}
