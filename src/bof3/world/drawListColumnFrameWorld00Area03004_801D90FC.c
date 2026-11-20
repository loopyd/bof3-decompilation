#include "bof3/world/area03004_internal.h"

/* @source 0x801D90FC
 * @behavior Draw the 0x8B by 0x14 inset header and fixed sprite frame shared by the rod
 * and lure columns. Draw the two side quads at x, y + 0x18 and x + 0x80, y +
 * 0x28 with heights 0x78 and 0x68, then draw the bottom sprite row.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawListColumnFrameWorld00Area03004(s16 x, s16 y) {
  s16 headerY;
  s32 spriteIndex;

  func_801AE3F0((u16)x, (u16)(y + 2), 0x8B, 0x14, 0, D_80144952);
  submitTpageDrawMode(9, 1);

  headerY = y;
  appendIndexedSpriteWorld00Area03004(0x3F, 1, x, headerY);
  appendIndexedSpriteWorld00Area03004(0x40, 1, (s16)(x + 8), headerY);
  appendIndexedSpriteWorld00Area03004(0x41, 1, (s16)(x + 0x10), headerY);

  for (spriteIndex = 0; spriteIndex < 8; spriteIndex++)
    appendIndexedSpriteWorld00Area03004(0x26, 1, (s16)(x + 0x28 + spriteIndex * 8), y);

  appendIndexedSpriteWorld00Area03004(0x42, 1, (s16)(x + 0x68), y);
  appendIndexedSpriteWorld00Area03004(0x43, 1, (s16)(x + 0x78), y);
  appendIndexedSpriteWorld00Area03004(0x44, 1, (s16)(x + 0x80), y);

  drawIndexedPanelQuadWithHeightWorld00Area03004(x, (s16)(y + 0x18), 0x78, 0);
  appendIndexedSpriteWorld00Area03004(0x46, 1, (s16)(x + 0x80), (s16)(y + 0x18));
  drawIndexedPanelQuadWithHeightWorld00Area03004((s16)(x + 0x80), (s16)(y + 0x28), 0x68, 4);

  appendIndexedSpriteWorld00Area03004(0x2A, 1, x, (s16)(y + 0x90));

  for (spriteIndex = 0; spriteIndex < 15; spriteIndex++)
    appendIndexedSpriteWorld00Area03004(0x2B, 1, (s16)(x + 8 + spriteIndex * 8), (s16)(y + 0x90));

  appendIndexedSpriteWorld00Area03004(0x45, 1, (s16)(x + 0x80), (s16)(y + 0x88));
}
