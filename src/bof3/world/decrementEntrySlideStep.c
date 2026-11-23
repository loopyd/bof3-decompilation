#include "bof3/world/area03004_internal.h"

/* @source 0x801DB808
 * @behavior Decrement work byte 9 modulo 256 and draw the panel pair and configured
 * sprite 2 at 128, 91. Draw the selected entry with amount zero and the fresh
 * countdown as its third, sliding-position argument. A fresh zero read
 * advances byte 3. Always step the pair sequence and append the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementEntrySlideStep(void) {
  u8* work;

  work = D_1F800044;
  work[9] = work[9] - 1;
  submitPanelPair(0, 78);
  configureSpriteClut(0x80, 91, D_80146888[D_801E3208].unk_02[4]);
  renderSlidingAmountSummary(D_80146888[D_801E3208].unk_02[4], 0, D_1F800044[9]);
  work = D_1F800044;
  if (work[9] == 0) {
    work[3]++;
  }
  func_8014D978();
  appendDimTile();
}
