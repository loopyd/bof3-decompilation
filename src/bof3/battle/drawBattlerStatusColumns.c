#include "bof3/battle/battle03_internal.h"

/* @source 0x80145F04 @kind table */
extern u8 D_80145F04[];
/* @source 0x80145F22 @kind unknown */
extern u8 D_80145F22[];
/* Byte views of the 0x24-stride UI owner records at 0x80148320: entry + 0x0D
 * carries its gauge widths at +0x1B/+0x1D, its sprite x offset at +0x1C and its
 * sprite length at +0x1E. */
/* @source 0x8014833B @kind unknown */
extern u8 D_8014833B[];
/* @source 0x8014833C @kind unknown */
extern u8 D_8014833C[];
/* @source 0x8014833D @kind unknown */
extern u8 D_8014833D[];
/* @source 0x8014833E @kind unknown */
extern u8 D_8014833E[];

/* @source 0x801D750C
 * @behavior Draws the battler status window at the panel anchor (arg0, arg1):
 * opens the 0x3C0-texture draw mode and emits the two quadrangle caps through
 * func_801D9AB4 at (arg0, arg1) and (arg0, arg1 + 0x18) and the body tile
 * through func_801D9C80 at (arg0, arg1 + 2), whose tile rows follow the
 * localBattlerCount byte 0x801462F0. It then draws one framed column per local
 * battler at x = arg0 + index * 0x5E: the row-9 colour tile and the textured
 * gauge through func_801D9E9C at (x + 0x14, arg1 + 0xE) with the byte +0x1B and
 * +0x1D widths of UI owner record 0x80148320 entry index + 0x0D, up to two
 * sprites for that entry's +0x1C offset while the battler record halfword +0x0E
 * (0x80145F22) is non-zero and the entry's +0x1E byte is non-zero, the record
 * text through func_8014FC90 at (x + 6, arg1 + 5) with the record halfword
 * +0x0C (0x80145F10) selecting style 2, two func_801DA078 accents at (x + 0x3A,
 * arg1 + 6) and (x + 4, arg1 + 0xE), and the record halfwords +0x14 (0x80145F18)
 * and +0x16 (0x80145F1A) as digit runs through drawDecimalGlyphRun at
 * (x + 0x48, arg1 + 0x0F) and (x + 0x48, arg1 + 8), the second styled 2 when its
 * halfword is zero or the record carries flag 0x4000 and otherwise 4 while that
 * halfword stays below the quarter of the record halfword +0x0E. The red, green
 * and blue channels of the column frame from (x + 2, arg1 + 2) through
 * (x + 0x5D, arg1 + 0x17) come from palette entry 1 of the CLUT bank byte
 * D_80144952.
 * @status partial
 * @match 66.57
 * @residual The column x base stays a strength-reduced multiply in this lift
 * where the original keeps one induction register advanced by 0x5E, so arg1
 * lands in s6 instead of s3 and the four localBattlerCount address
 * materialisations are not shared. The first 0x8014833C read materialises the
 * symbol value where the original folds it into the %lo displacement, and the
 * two shape-spelled record offsets add 0x0D after scaling where the original
 * adds it before. The body measures 338 instructions against the original 333.
 */
void drawBattlerStatusColumns(s32 arg0, s32 arg1) {
  s32 palette_offset;
  u16 palette;
  u8  red;
  u8  green;
  u8  blue;
  s32 index;
  u16 flags;
  u16 count;
  u8  style;
  s32 y;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x3c0, 0), 0);
  func_8014E5A0(1, 0x0c);
  func_801D9AB4((s16)arg0, (s16)arg1, (localBattlerCount * 2 + 0x0a) & 0xfe, 1);
  func_801D9AB4((s16)arg0, (s16)(arg1 + 0x18),
                (localBattlerCount * 2 + 0x0b) & 0xff, 1);
  func_801D9C80((s16)arg0, (s16)(arg1 + 2), (localBattlerCount + 0x0d) & 0xff,
                1);
  palette_offset = ((D_80144952 * 2) + 1) * 32;
  palette = *(u16*)(PSX_PTR(u8, 0x80033A08u) + palette_offset);
  red = (palette & 0x1f) << 3;
  green = (palette >> 2) & 0xf8;
  blue = (palette >> 7) & 0xf8;
  y = arg1 + 0x0e;
  index = 0;
  if (localBattlerCount != 0) {
    do {
      drawColorTilePrim((s16)(arg0 + index * 0x5e + 0x14), (s16)y, 9, 0, 0);
      func_801D9E9C((s16)(arg0 + index * 0x5e + 0x14), (s16)y,
                    D_8014833B[((u8)index + 0x0d) * 0x24],
                    D_8014833D[((u8)index + 0x0d) * 0x24], 0);
      if (*(u16*)&D_80145F22[(u8)index * 0x140] != 0) {
        drawSpritePrimTpage1(
            (s16)(arg0 + index * 0x5e + 0x14), (s16)y,
            (s16)(arg0 + index * 0x5e +
                  (D_8014833C[((u8)index + 0x0d) * 0x24] + 0x14)),
            (s16)y, 0x64, 0, 0xc8);
        if (D_8014833E[((u8)index + 0x0d) * 0x24] != 0) {
          drawSpritePrimTpage1(
              (s16)(arg0 + index * 0x5e +
                    (D_8014833C[((u8)index + 0x0d) * 0x24] + 0x14)),
              (s16)y,
              (s16)(D_8014833C[((u8)index + 0x0d) * 0x24] +
                    (arg0 + index * 0x5e + 0x14) +
                    D_8014833E[((u8)index + 0x0d) * 0x24]),
              (s16)y, 0xc8, 0, 0);
        }
      }
      flags = *(volatile u16*)&D_80145F10[(u8)index * 0x140];
      func_8014FC90((s16)(arg0 + index * 0x5e + 6), (s16)(arg1 + 5),
                    (flags & 0x4000) ? 2 : ((flags & 0xbfc) != 0), 5,
                    &D_80145F04[(u8)index * 0x140]);
      func_801DA078((u16)(arg0 + index * 0x5e + 0x3a), (u16)(arg1 + 6), 1);
      func_801DA078((u16)(arg0 + index * 0x5e + 4), (u16)y, 0);
      flags = *(volatile u16*)&D_80145F10[(u8)index * 0x140];
      drawDecimalGlyphRun((s16)(arg0 + index * 0x5e + 0x48), (s16)(arg1 + 0x0f),
                          (flags & 0x4000) ? 2 : ((flags >> 11) & 4),
                          *(volatile u16*)&D_80145F18[(u8)index * 0x140]);
      count = *(volatile u16*)&D_80145F1A[(u8)index * 0x140];
      style = 2;
      if (count != 0) {
        if ((*(volatile u16*)&D_80145F10[(u8)index * 0x140] & 0x4000) == 0) {
          style = (count < (*(u16*)&D_80145F22[(u8)index * 0x140] >> 2)) << 2;
        }
      }
      drawDecimalGlyphRun((s16)(arg0 + index * 0x5e + 0x48), (s16)(arg1 + 8),
                          style, *(volatile u16*)&D_80145F1A[(u8)index * 0x140]);
      drawSpritePrimTpage1((s16)(arg0 + index * 0x5e + 2), (s16)(arg1 + 2),
                           (s16)(arg0 + index * 0x5e + 0x5d),
                           (s16)(arg1 + 2), red, green, blue);
      drawSpritePrimTpage1((s16)(arg0 + index * 0x5e + 2), (s16)(arg1 + 3),
                           (s16)(arg0 + index * 0x5e + 2),
                           (s16)(arg1 + 0x17), red, green, blue);
      drawSpritePrimTpage0((s16)(arg0 + index * 0x5e + 0x5d), (s16)(arg1 + 2),
                           (s16)(arg0 + index * 0x5e + 0x5d),
                           (s16)(arg1 + 0x17), red, green, blue);
      drawSpritePrimTpage0((s16)(arg0 + index * 0x5e + 2), (s16)(arg1 + 0x17),
                           (s16)(arg0 + index * 0x5e + 0x5d),
                           (s16)(arg1 + 0x17), red, green, blue);
      index++;
    } while ((u8)index < localBattlerCount);
  }
}
