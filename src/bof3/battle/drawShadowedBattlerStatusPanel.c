#include "bof3/battle/battle03_internal.h"

/* @source 0x801D8690
 * @behavior Draws the battle status panel of the battler whose panel index is
 * arg2 at the anchor (arg0, arg1) over an offset copy of itself: under
 * GetTPage(0, 0, 0x3C0, 0) it first emits the D_801EAE50 row-8 panel tile at
 * (arg0 + 4, arg1 + 8) and the two D_801EAD30 quadrangle caps of rows 2 and 3
 * at (arg0 + 4, arg1 + 6) and (arg0 + 4, arg1 + 0x19) in palette entry 1, then
 * under GetTPage(0, 1, 0x3C0, 0) the panel itself in opaque palette entry 0:
 * the row-8 tile at (arg0, arg1 + 2), the row-9 colour tile at
 * (arg0 + 6, arg1 + 0xB) and the caps of rows 2 and 3 at (arg0, arg1) and
 * (arg0, arg1 + 0x13). The red/green/blue channels come from palette entry 1 of
 * the CLUT bank byte D_80144952. It reads byte +0x7F of the 0x118-byte
 * D_801EB630 record selected by arg2 - 3: when that byte reads 1 it submits the
 * record's textured gauge through func_801D9E9C at (arg0 + 6, arg1 + 0xA) with
 * the current panel object bytes +0x0B/+0x0D as the filled and total widths,
 * otherwise it blanks the numeric field through drawDecimalGlyphRun at
 * (arg0 + 0x24, arg1 + 0xC), and it then always draws the record text pointer
 * at +0x74 through func_8014FC90 at (arg0 + 4, arg1 + 3). Finally it outlines
 * the box from (arg0 + 2, arg1 + 2) through (arg0 + 0x49, arg1 + 0x12): the top
 * and left edges as texture-page-1 sprite prims in that palette colour and the
 * right and bottom edges as flat line prims in the same channels lowered by
 * 0x10 and clamped to 0 through 0xFF. Its only in-image caller, func_801E99AC,
 * feeds it the active panel halfwords +0x04/+0x06 and panel byte +0x0A.
 * @status partial
 * @match 92.06
 * @residual Live diff 255/277 instructions at an unchanged 1108-byte length and
 * frame; the 22 unmatched words are the callee-saved register names of the two
 * palette colour locals and the second anchor copy, which this build holds in
 * s7, s6 and s5 where the original holds them in s6, s5 and s7. Instruction
 * multiset, order, frame size, byte count and every immediate already agree.
 * Measured families that left that rotation unchanged: index width (u8, u16,
 * s32) and its placement, the anchor copies placed early, at the phase-B block
 * and at the branch, arm operands written through the copies and through the
 * formals, colour statement and declaration order, and an inlined palette
 * offset.
 */
void drawShadowedBattlerStatusPanel(s32 arg0, s32 arg1, s32 arg2) {
  s32 palette_offset;
  u16 palette;
  s16 red;
  s16 green;
  s16 blue;
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
  func_801D9C80((s16)(arg0 + 4), (s16)(arg1 + 8), 8, 1);
  func_801D9AB4((s16)(arg0 + 4), (s16)(arg1 + 6), 2, 1);
  func_801D9AB4((s16)(arg0 + 4), (s16)(arg1 + 0x19), 3, 1);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x3c0, 0), 0);
  func_8014E5A0(1, 0x0c);
  func_801D9C80((s16)arg0, (s16)(arg1 + 2), 8, 0);
  drawColorTilePrim((s16)(arg0 + 6), (s16)(arg1 + 0xb), 9, 0, 0);
  func_801D9AB4((s16)arg0, (s16)arg1, 2, 0);
  func_801D9AB4((s16)arg0, (s16)(arg1 + 0x13), 3, 0);
  x = arg0;
  y = arg1;
  if (D_801EB6AF[index * 0x118] == 1) {
    func_801D9E9C((s16)(arg0 + 6), (s16)(arg1 + 0xa), D_80148648[0xb],
                  D_80148648[0xd], 1);
  } else {
    drawDecimalGlyphRun((s16)(arg0 + 0x24), (s16)(arg1 + 0xc), 0, 0xffff);
  }
  func_8014FC90((s16)(x + 4), (s16)(y + 3), 0, 8,
                &D_801EB6A4[index * 0x118]);
  drawSpritePrimTpage1((s16)(x + 2), (s16)(y + 2), (s16)(x + 0x49),
                       (s16)(y + 2), red, green, blue);
  drawSpritePrimTpage1((s16)(x + 2), (s16)(y + 3), (s16)(x + 2),
                       (s16)(y + 0x12), red, green, blue);
  red -= 0x10;
  green -= 0x10;
  blue -= 0x10;
  red = red < 0 ? 0 : red < 0x100 ? red : 0xff;
  green = green < 0 ? 0 : green < 0x100 ? green : 0xff;
  blue = blue < 0 ? 0 : blue < 0x100 ? blue : 0xff;
  drawFlatLinePrim((s16)(x + 0x49), (s16)(y + 2), (s16)(x + 0x49),
                   (s16)(y + 0x12), red, green, blue);
  drawFlatLinePrim((s16)(x + 2), (s16)(y + 0x12), (s16)(x + 0x49),
                   (s16)(y + 0x12), red, green, blue);
}
