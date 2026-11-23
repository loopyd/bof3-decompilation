#include "bof3/world/area03004_internal.h"

/* @source 0x801DBA94
 * @behavior Decrement work byte 9 and draw the panel row with its icon row at y = 30 *
 * count + 156 and the conditional extra icon. A fresh zero read advances byte
 * 3. The configured sprite uses y = 91. Always step the pair sequence and
 * append the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The mid-function count read for renderSummaryThresholdIcons goes through the published
 * D_1F800044 symbol instead of the reassigned `slideWork` local: only the direct
 * global read keeps the reloaded cursor in $v0, while the ordinary local
 * reassignment allocates it to $v1 and shifts the three reload instructions.
 */
void decrementIconRowSlide(void) {
  u8* slideWork;

  slideWork = D_1F800044;
  slideWork[9] = slideWork[9] - 1;
  WORLD00_AREA030_DRAW_PANEL_ROW((s16)(D_1F800044[9] * 30 + 156));
  renderExtraIconEnabled();
  slideWork = D_1F800044;
  if (slideWork[9] == 0) {
    slideWork[3]++;
  }
  func_8014D978();
  appendDimTile();
}
