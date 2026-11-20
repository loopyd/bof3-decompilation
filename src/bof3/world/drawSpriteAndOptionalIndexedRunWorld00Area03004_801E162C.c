#include "bof3/world/area03004_internal.h"

/* @source 0x801E162C
 * @behavior Select draw mode 3/1 and draw sprite 0x0C at x and y. Unless runIndex is
 * 0xFFFF, read its unsigned halfword offset from the main-RAM table at
 * 0x80010000 and draw the indexed byte stream at x + 10 and y + 8 with colour
 * 0 and flag 0xFF.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpriteAndOptionalIndexedRunWorld00Area03004(s32 x, s32 y, u16 runIndex) {
  submitTpageDrawMode(3, 1);
  appendIndexedSpriteWorld00Area03004(0xC, 1, (s16)x, (s16)y);

  if (runIndex != 0xFFFF) {
    drawEncodedTextAt((s16)(x + 0xA), (s16)(y + 8), 0, 0xFF,
                  0x80010000u + PSX_PTR(u16, 0x80010000u)[runIndex]);
  }
}
