#include "bof3/world/area03004_internal.h"

/* @source 0x801DBD6C
 * @behavior Decrement work byte 9 and draw the fixed icon row and configured sprite at y
 * 91. For work byte 0xB = 1 or 2, draw sprite 0x0C with run 67 or 66 at x 8, y
 * = 30 * count + 148. At zero, mode 1 clears byte 6 and selects step 12; mode
 * 2 advances byte 3. Always step the pair sequence and append the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementRunSlideAndSelectWorkStepAtZeroWorld00Area03004(void) {
  u8* slideWork;

  slideWork = D_1F800044;
  slideWork[9] = slideWork[9] - 1;
  WORLD00_AREA030_DRAW_PANEL_ROW(156);
  slideWork = D_1F800044;
  if (slideWork[0xB] == 1) {
    drawSpriteAndOptionalIndexedRunWorld00Area03004(8, slideWork[9] * 30 + 148, 67);
  } else if (slideWork[0xB] == 2) {
    drawSpriteAndOptionalIndexedRunWorld00Area03004(8, slideWork[9] * 30 + 148, 66);
  }
  slideWork = D_1F800044;
  if (slideWork[9] == 0) {
    if (slideWork[0xB] == 1) {
      slideWork[6] = 0;
      slideWork = D_1F800044;
      slideWork[3] = 0x0C;
    } else if (slideWork[0xB] == 2) {
      slideWork[3]++;
    }
  }
  func_8014D978();
  appendDimTile();
}
