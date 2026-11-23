#include "bof3/world/area03004_internal.h"

/* @source 0x801E1758
 * @behavior draws the compact AREA030 panel at the requested origin: submits the 0x64 by
 * 0x38 inset panel rectangle through drawFramedTexturedPanel with the shared
 * CLUT-bank byte D_80144952 as the trailing argument, selects graphics mode
 * 9/1, draws the 0x25 sprite at x, eight 0x26 sprites spaced eight pixels from
 * x + 0x20, the 0x27 and 0x28 sprites at x + 0x48 and x + 0x60, the two
 * renderIndexedQuadHeight text rows of width 0x20 at
 * (x, y + 0x18) and (x + 0x60, y + 0x18), the 0x30 sprite at x on the y + 0x38
 * row, eleven 0x31 sprites spaced eight pixels from x + 8 on that row and the
 * 0x32 sprite at x + 0x60 on that row, and finally the three main-RAM
 * offset-table textures (the 0x80010000 base plus the selected halfword) at (x
 * + 0x23, y + 6) and at (x + 0xA, y + 0x19) and (x + 0xA, y + 0x29), each with
 * colour 0 and flag 0xFF through drawEncodedTextAt.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawArea030CompactPanel(s16 x, s16 y) {
  s32 i;

  drawFramedTexturedPanel((u16)x, (u16)(y + 1), 0x64, 0x38, 0x80, D_80144952);
  submitTpageDrawMode(9, 1);
  appendIndexedSprite(0x25, 1, x, y);

  for (i = 0; i < 8; i++)
    appendIndexedSprite(0x26, 1, (s16)(x + 0x20 + i * 8), y);
  appendIndexedSprite(0x27, 1, (s16)(x + 0x48), y);
  appendIndexedSprite(0x28, 1, (s16)(x + 0x60), y);

  renderIndexedQuadHeight(x, (s16)(y + 0x18), 0x20, 0);
  renderIndexedQuadHeight((s16)(x + 0x60), (s16)(y + 0x18), 0x20, 1);

  appendIndexedSprite(0x30, 1, x, (s16)(y + 0x38));
  for (i = 0; i < 0xB; i++)
    appendIndexedSprite(0x31, 1, (s16)(x + 8 + i * 8), (s16)(y + 0x38));
  appendIndexedSprite(0x32, 1, (s16)(x + 0x60), (s16)(y + 0x38));

  drawEncodedTextAt((s16)(x + 0x23), (s16)(y + 6), 0, 0xFF,
                0x80010000u + PSX_PTR(u16, 0x80010000u)[0x3D]);
  drawEncodedTextAt((s16)(x + 0xA), (s16)(y + 0x19), 0, 0xFF,
                0x80010000u + PSX_PTR(u16, 0x80010000u)[0x3E]);
  drawEncodedTextAt((s16)(x + 0xA), (s16)(y + 0x29), 0, 0xFF,
                0x80010000u + PSX_PTR(u16, 0x80010000u)[0x3F]);
}
