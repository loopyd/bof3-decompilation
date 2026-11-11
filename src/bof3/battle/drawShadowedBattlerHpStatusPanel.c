#include "bof3/battle/battle03_internal.h"

/* Record base of the 0x140-stride battler rows this panel reads: the name
 * string at +0x00, the flag halfword at +0x0C, the current HP halfword at
 * +0x14 and the maximum HP halfword at +0x1C.
 * @source 0x80145F04 @kind table */
extern u8 D_80145F04[];

/* Separator string of this payload's leading data blob, formatted into the
 * shared buffer D_80145AD4 between the current and maximum HP digit runs.
 * @source 0x801D0C6C @kind rodata */
extern const char D_801D0C6C[];

/* @source 0x801D8DF8
 * @behavior Draws the HP status panel of the battler whose panel index is arg2
 * at the anchor (arg0, arg1) over an offset shadow copy of itself: under
 * GetTPage(0, 0, 0x3C0, 0) it emits the D_801EAE50 row-8 panel tile at
 * (arg0 + 4, arg1 + 8) and the D_801EAD30 quadrangle caps of rows 2 and 3 at
 * (arg0 + 4, arg1 + 6) and (arg0 + 4, arg1 + 0x19) in palette entry 1, then
 * under GetTPage(0, 1, 0x3C0, 0) the panel itself in opaque palette entry 0:
 * the row-8 tile at (arg0, arg1 + 2) and the caps of rows 2 and 3 at (arg0,
 * arg1) and (arg0, arg1 + 0x13). The red, green and blue channels come from
 * palette entry 1 of the CLUT bank byte D_80144952. It then reads the record
 * halfword +0x0C of the 0x140-stride row D_80145F04 selected by arg2 and draws
 * that row name string at +0x00 through func_8014FC90 at (arg0 + 4, arg1 + 2),
 * styled 2 while the halfword carries bit 0x4000, 1 while any of its 0xBFC
 * bits is set and 0 otherwise; one emitBattlerStatusAccentSprite accent at
 * (arg0 + 2, arg1 + 0x0A); the row current HP halfword +0x14 through
 * drawDecimalGlyphRun at (arg0 + 0x14, arg1 + 0xC), styled 2 while that same
 * halfword carries bit 0x4000 and 4 on its bit 11 otherwise; the "/" separator
 * at 0x801D0C6C formatted through sprintf into D_80145AD4 together with that
 * halfword shifted 13 and drawn through func_80150098 at (arg0 + 0x28,
 * arg1 + 0x0A) in the shade (halfword >> 13) & 2; and the row maximum HP
 * halfword +0x1C through drawDecimalGlyphRun at (arg0 + 0x30, arg1 + 0xC) in
 * that same shade. Finally it outlines the box from (arg0 + 2, arg1 + 2)
 * through (arg0 + 0x49, arg1 + 0x12): the top and left edges as
 * texture-page-1 sprite prims in the palette channels and the right and bottom
 * edges as flat line prims in those channels lowered by 0x10 and clamped to 0
 * through 0xFF. Its only in-image caller, func_801E9FDC, feeds it the active
 * panel halfwords +0x04/+0x06 and panel byte +0x0A.
 * @status partial
 * @match 72.53
 * @residual Live comparison matches 235/323 original instructions; the build
 * has 324 instructions and 1296 bytes against the original 1292 bytes. The
 * first difference is the saved-register assignment in the prologue. Entry
 * flags have a separate lifetime from the later volatile reloads; register
 * allocation and style-arm layout remain non-exact.
 */
void drawShadowedBattlerHpStatusPanel(s32 arg0, s32 arg1, u32 arg2) {
  s32 palette_offset;
  u16 palette;
  s16 red;
  s16 green;
  s16 blue;
  u16 flags;
  u16 entry_flags;
  u8  spriteGreen;
  u8  spriteBlue;
  s32 x;
  s32 y;
  s32 style;
  s32 offset;
  u32 entry_offset;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x3c0, 0), 0);
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
  func_801D9AB4((s16)arg0, (s16)arg1, 2, 0);
  func_801D9AB4((s16)arg0, (s16)(arg1 + 0x13), 3, 0);
  x = arg0;
  y = arg1;
  entry_offset = (u8)arg2 * 0x140;
  entry_flags = *(volatile u16*)&D_80145F10[entry_offset];
  style = (entry_flags & 0x4000) ? 2 : ((entry_flags & 0xbfc) != 0);
  offset = (u8)arg2 * 0x140;
  func_8014FC90((s16)(x + 4), (s16)(y + 2), style, 5,
                &D_80145F04[offset]);
  flags = *(volatile u16*)&D_80145F10[offset];
  style = (flags & 0x4000) ? 2 : ((flags >> 11) & 4);
  emitBattlerStatusAccentSprite((u16)(x + 2), (u16)(y + 0xa), 0);
  spriteGreen = green;
  spriteBlue = blue;
  green -= 0x10;
  blue -= 0x10;
  drawDecimalGlyphRun((s16)(x + 0x14), (s16)(y + 0xc), style,
                      *(volatile u16*)&D_80145F18[offset]);
  flags = *(volatile u16*)&D_80145F10[offset];
  sprintf((char*)D_80145AD4, D_801D0C6C, flags >> 13);
  style = (flags >> 13) & 2;
  func_80150098((s16)(x + 0x28), (s16)(y + 0xa), (u8)style, D_80145AD4);
  drawDecimalGlyphRun((s16)(x + 0x30), (s16)(y + 0xc), (u8)style,
                      *(volatile u16*)&D_80145F20[offset]);
  drawSpritePrimTpage1((s16)(x + 2), (s16)(y + 2), (s16)(x + 0x49),
                       (s16)(y + 2), red, spriteGreen, spriteBlue);
  drawSpritePrimTpage1((s16)(x + 2), (s16)(y + 3), (s16)(x + 2),
                       (s16)(y + 0x12), red, spriteGreen, spriteBlue);
  red -= 0x10;
  red = red < 0 ? 0 : red < 0x100 ? red : 0xff;
  green = green < 0 ? 0 : green < 0x100 ? green : 0xff;
  blue = blue < 0 ? 0 : blue < 0x100 ? blue : 0xff;
  drawFlatLinePrim((s16)(x + 0x49), (s16)(y + 2), (s16)(x + 0x49),
                   (s16)(y + 0x12), red, green, blue);
  drawFlatLinePrim((s16)(x + 2), (s16)(y + 0x12), (s16)(x + 0x49),
                   (s16)(y + 0x12), red, green, blue);
}
