#include "bof3/world/area03004_internal.h"

/* @source 0x801D7D68
 * @behavior draws one 0x28 by 0x10 AREA030 status-panel cell at the requested origin:
 * submits the inset panel rectangle through drawFramedTexturedPanel with the
 * shared CLUT-bank byte D_80144952 as its trailing argument, selects graphics
 * mode 8/1, then draws the groupIndex-selected spriteBase (groupIndex*4 +
 * 0x1D) at (x, y), the (groupIndex*4 + 0x1E) spriteBase on the y + 0x10
 * lowerY, four (groupIndex*4 + 0x1F) sprites spaced eight pixels from x + 8 on
 * that lowerY and finally the (groupIndex*4 + 0x20) spriteBase at (x + 0x28, y
 * + 0x10).
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawGroupedCellSpritesWorld00Area03004(s16 x, s16 y, u8 groupIndex) {
  s16 lowerY;
  s32 spriteBase;
  s32 cellIndex;

  drawFramedTexturedPanel((u16)(x + 2), (u16)(y + 2), 0x28, 0x10, 0, D_80144952);
  submitTpageDrawMode(8, 1);
  spriteBase = groupIndex * 4;
  appendIndexedSpriteWorld00Area03004((spriteBase + 0x1D) & 0xFF, 1, x, y);

  lowerY = y + 0x10;
  appendIndexedSpriteWorld00Area03004((spriteBase + 0x1E) & 0xFE, 1, x, lowerY);

  for (cellIndex = 0; cellIndex < 4; cellIndex++)
    appendIndexedSpriteWorld00Area03004((spriteBase + 0x1F) & 0xFF, 1, (s16)(x + (cellIndex + 1) * 8), lowerY);

  /* The final cell index is recomputed from the groupIndex instead of reusing the
   * loop's index local: the original re-derives it from the groupIndex byte here,
   * the register holding the loop value having been consumed in place. */
  appendIndexedSpriteWorld00Area03004((groupIndex * 4 + 0x20) & 0xFC, 1, (s16)(x + 0x28), (s16)(y + 0x10));
}
