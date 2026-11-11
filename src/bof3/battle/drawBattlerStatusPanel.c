#include "bof3/battle/battle03_internal.h"

/* @source 0x801D8AE4
 * @behavior Draws the battle status panel of the battler whose panel index is
 * arg2 at the anchor (arg0, arg1): opens the 0x3C0-texture draw mode, emits the
 * D_801EAE50 row-8 panel tile at (arg0, arg1 + 2), the row-9 colour tile at
 * (arg0 + 6, arg1 + 0xB) and the two quadrangle caps from D_801EAD30 rows 2 and
 * 3 at (arg0, arg1) and (arg0, arg1 + 0x13); the red/green/blue channels come
 * from palette entry 1 of the CLUT bank byte D_80144952. It then reads byte
 * +0x7F of the 0x118-byte D_801EB630 record selected by arg2 - 3: when that
 * byte reads 1 it submits the record's textured gauge through
 * drawBattlerStatusGaugeBar at (arg0 + 6, arg1 + 0xA) with the current panel
 * object bytes +0x0B/+0x0D as the two gauge widths, otherwise it blanks the
 * numeric field through drawDecimalGlyphRun at (arg0 + 0x24, arg1 + 0xC). When func_801DA69C accepts
 * that index it draws the record's text pointer at +0x74 through func_8014FC90
 * at (arg0 + 4, arg1 + 3), and finally outlines the box from
 * (arg0 + 2, arg1 + 2) through (arg0 + 0x49, arg1 + 0x12) with four
 * semi-transparent sprite prims: the top and left edges from texture page 1 and
 * the right and bottom edges from texture page 0. The only caller,
 * func_801E9910, feeds it the active panel halfwords +0x04/+0x06 and panel byte
 * +0x0A.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawBattlerStatusPanel(s32 arg0, s32 arg1, s32 arg2) {
  s32 palette_offset;
  u16 palette;
  u8  red;
  u8  green;
  u8  blue;
  u8  index;
  s32 x;
  s32 y;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x3c0, 0), 0);
  index = arg2 - 3;
  func_8014E5A0(1, 0x0c);
  palette_offset = ((D_80144952 * 2) + 1) * 32;
  palette = *(u16*)(PSX_PTR(u8, 0x80033A08u) + palette_offset);
  red = (palette & 0x1f) << 3;
  green = (palette >> 2) & 0xf8;
  blue = (palette >> 7) & 0xf8;
  func_801D9C80((s16)arg0, (s16)(arg1 + 2), 8, 1);
  drawColorTilePrim((s16)(arg0 + 6), (s16)(arg1 + 0xb), 9, 0, 0);
  func_801D9AB4((s16)arg0, (s16)arg1, 2, 1);
  func_801D9AB4((s16)arg0, (s16)(arg1 + 0x13), 3, 1);
  /* Preserve the anchors for the outline; reuse the consumed formals for
   * the final edge coordinates below so the two phases stay distinct. */
  x = arg0;
  y = arg1;
  if (D_801EB6AF[index * 0x118] == 1) {
    drawBattlerStatusGaugeBar((s16)(arg0 + 6), (s16)(arg1 + 0xa),
                              D_80148648[0xb], D_80148648[0xd], 1);
  } else {
    drawDecimalGlyphRun((s16)(arg0 + 0x24), (s16)(arg1 + 0xc), 0, 0xffff);
  }
  if (func_801DA69C(index) != 0) {
    func_8014FC90((s16)(x + 4), (s16)(y + 3), 0, 8,
                  &D_801EB6A4[index * 0x118]);
  }
  drawSpritePrimTpage1((s16)(x + 2), (s16)(y + 2), (s16)(x + 0x49),
                       (s16)(y + 2), red, green, blue);
  drawSpritePrimTpage1((s16)(x + 2), (s16)(y + 3), (s16)(x + 2),
                       (s16)(y + 0x12), red, green, blue);
  drawSpritePrimTpage0((s16)(x + 0x49), (s16)(y + 2), (s16)(x + 0x49),
                       (s16)(y + 0x12), red, green, blue);
  arg0 = x + 0x49;
  arg1 = y + 0x12;
  drawSpritePrimTpage0((s16)(x + 2), (s16)arg1, (s16)arg0,
                       (s16)arg1, red, green, blue);
}
