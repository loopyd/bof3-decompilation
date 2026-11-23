#include "bof3/world/area03004_internal.h"

/* @source 0x801D7784
 * @behavior draws the AREA030 status panel icon row and its framed sprite
 * columns at the requested origin: selects graphics mode 9/1, appends sprite
 * 0x25 at (x, y), 0x17 sprites 0x26 spaced 8 pixels from x + 0x20, 0x27 at
 * x + 0xD8 and 0x28 at x + 0xF0, submits the frame quads at x and x + 0xF0
 * through drawIndexedPanelQuadWithHeightWorld00Area03004, then draws 0x2A at x, 0x17 sprites 0x2B from x + 8,
 * 0x2C at x + 0xC0, three sprites 0x2D from x + 0xC8 and finally 0x2E at
 * x + 0xE0 and 0x2F at x + 0xE7, all on the y + 0x90 row.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWidePanelFrameWorld00Area03004(s16 x, s16 y) {
  s32 spriteIndex;

  submitTpageDrawMode(9, 1);
  appendIndexedSprite(0x25, 1, x, y);

  for (spriteIndex = 0; spriteIndex < 0x17; spriteIndex++)
    appendIndexedSprite(0x26, 1, (s16)(x + 0x20 + spriteIndex * 8), y);

  appendIndexedSprite(0x27, 1, (s16)(x + 0xD8), y);
  appendIndexedSprite(0x28, 1, (s16)(x + 0xF0), y);
  drawIndexedPanelQuadWithHeightWorld00Area03004(x, (s16)(y + 0x18), 0x78, 0);
  drawIndexedPanelQuadWithHeightWorld00Area03004((s16)(x + 0xF0), (s16)(y + 0x18), 0x78, 1);
  appendIndexedSprite(0x2A, 1, x, (s16)(y + 0x90));

  for (spriteIndex = 0; spriteIndex < 0x17; spriteIndex++)
    appendIndexedSprite(0x2B, 1, (s16)(x + 8 + spriteIndex * 8), (s16)(y + 0x90));

  appendIndexedSprite(0x2C, 1, (s16)(x + 0xC0), (s16)(y + 0x90));

  for (spriteIndex = 0; spriteIndex < 3; spriteIndex++)
    appendIndexedSprite(0x2D, 1, (s16)(x + 0xC8 + spriteIndex * 8), (s16)(y + 0x90));

  appendIndexedSprite(0x2E, 1, (s16)(x + 0xE0), (s16)(y + 0x90));
  appendIndexedSprite(0x2F, 1, (s16)(x + 0xE7), (s16)(y + 0x90));
}
