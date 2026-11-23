#include "bof3/world/area03004_internal.h"

/* @source 0x801DB738
 * @behavior Decrement work byte 9 modulo 256 and draw the panel pair at y 78. Configure
 * sprite 2 at x = 128 + 48 * a fresh countdown, y = 91, using selected-record
 * byte 6 as the packed selector. A fresh zero read reloads countdown 4 and
 * advances byte 3. Always step the pair sequence and append the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementEntrySpriteCountdown(void) {
  u8* countdownWork;

  countdownWork = D_1F800044;
  countdownWork[9] = countdownWork[9] - 1;
  submitPanelPair(0, 78);
  configureSpriteClut((s16)(D_1F800044[9] * 0x30 + 0x80), 91,
                      D_80146888[D_801E3208].unk_02[4]);
  countdownWork = D_1F800044;
  if (countdownWork[9] == 0) {
    countdownWork[9] = 4;
    countdownWork = D_1F800044;
    countdownWork[3]++;
  }
  func_8014D978();
  appendDimTile();
}
