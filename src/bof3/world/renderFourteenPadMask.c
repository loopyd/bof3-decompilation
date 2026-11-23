#include "bof3/world/area03004_internal.h"

/* @source 0x801DBEDC
 * @behavior Draw the entry panel row, configured sprite 2 at 128, 91, and sprite 0x0C
 * followed by indexed run 66 at 8, 148. A nonzero 0x50 input mask selects
 * handler byte 3 = 14. Always step the pair sequence and append the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderFourteenPadMask(void) {
  WORLD00_AREA030_DRAW_PANEL_ROW(156);
  renderSpriteOptionalIndexed(8, 148, 66);
  if ((D_80145AA8 & 0x50) != 0) {
    /* The fixed-address scratch cursor view, not the named D_1F800044 symbol:
     * only this representation reproduces the original's constant-in-delay-slot
     * store schedule (li v0,14 before the cursor load). */
    WORLD00_AREA030_SCRATCH_PTR[3] = 0x0E;
  }
  func_8014D978();
  appendDimTile();
}
