#include "bof3/battle/battle03_internal.h"

/* @source 0x801D8090
 * @behavior Draws a narrow battle panel strip at the anchor (arg0, arg1): opens
 * the 0x3C0-texture draw mode, emits the 0x48 x 0x0F panel tile (D_801EAE50 row
 * 6) at (arg0, arg1 + 2) and the two quadrangle caps from D_801EAD30 rows 4 and
 * 5 at (arg0, arg1) and (arg0, arg1 + 0x11), then outlines the rectangle inset
 * to (arg0 + 2, arg1 + 2) through (arg0 + 0x45, arg1 + 0x10) with four
 * semi-transparent sprite prims: the top and left edges from texture page 1 and
 * the right and bottom edges from texture page 0. The red/green/blue channels
 * come from palette entry 1 of the CLUT bank byte D_80144952. The instruction
 * stream is identical to the wide strip at 0x801D7EB0 and the message strip at
 * 0x801D8270 except for the panel tile row, the two cap rows and the right
 * inset, so the three siblings outline panels of three tile widths. No extracted
 * image references 0x801D8090 -- neither a jal nor a pointer word in the EXE or
 * in any overlay payload -- so the panel that consumes this variant, and
 * therefore its semantic name, remains unresolved.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D8090(s32 arg0, s32 arg1) {
  s32 palette_offset;
  u16 palette;
  s32 red;
  s32 green;
  s32 blue;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x3c0, 0), 0);
  func_8014E5A0(1, 0x0c);
  palette_offset = ((D_80144952 * 2) + 1) * 32;
  palette = *(u16*)(PSX_PTR(u8, 0x80033A08u) + palette_offset);
  red = (palette & 0x1f) << 3;
  green = (palette >> 2) & 0xf8;
  blue = (palette >> 7) & 0xf8;
  func_801D9C80((s16)arg0, (s16)(arg1 + 2), 6, 1);
  func_801D9AB4((s16)arg0, (s16)arg1, 4, 1);
  func_801D9AB4((s16)arg0, (s16)(arg1 + 0x11), 5, 1);
  drawSpritePrimTpage1((s16)(arg0 + 2), (s16)(arg1 + 2), (s16)(arg0 + 0x45),
                       (s16)(arg1 + 2), red, green, blue);
  drawSpritePrimTpage1((s16)(arg0 + 2), (s16)(arg1 + 3), (s16)(arg0 + 2),
                       (s16)(arg1 + 0x10), red, green, blue);
  drawSpritePrimTpage0((s16)(arg0 + 0x45), (s16)(arg1 + 2), (s16)(arg0 + 0x45),
                       (s16)(arg1 + 0x10), red, green, blue);
  drawSpritePrimTpage0((s16)(arg0 + 2), (s16)(arg1 + 0x10), (s16)(arg0 + 0x45),
                       (s16)(arg1 + 0x10), red, green, blue);
}
