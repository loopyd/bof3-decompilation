#include "bof3/world/area03004_internal.h"

/* @source 0x801DBFC8
 * @behavior Draw the fixed icon row at y 156, configured sprite 2 at 128, 91, and sprite
 * 0x0C followed by indexed run 67 at 8, 148. If D_80145AA8 is nonzero, clear
 * work byte 6 and advance byte 3. Always step the pair sequence and append the
 * dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFixedIconRowAndRunThenAdvanceStepOnInputWorld00Area03004(void) {
  WORLD00_AREA030_DRAW_PANEL_ROW(156);
  drawSpriteAndOptionalIndexedRunWorld00Area03004(8, 148, 67);
  if (D_80145AA8 != 0) {
    D_1F800044[6] = 0;
    D_1F800044[3]++;
  }
  func_8014D978();
  appendDimTile();
}
