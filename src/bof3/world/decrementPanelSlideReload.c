#include "bof3/world/area03004_internal.h"

/* @source 0x801DB620
 * @behavior Decrement work byte 9 and draw the panel pair at horizontal offset -40 *
 * count, y 78. At zero, request cue 0x20C or 0x20B according to the selected
 * record's byte 0x1D, reload countdown 4 and advance byte 3. Always step the
 * pair sequence and append the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementPanelSlideReload(void) {
  u8* slideWork;
  u32 recordIndex;
  u32 cue;

  slideWork = D_1F800044;
  slideWork[9] = slideWork[9] - 1;
  submitPanelPair((s16)(-40 * D_1F800044[9]), 78);
  if (D_1F800044[9] == 0) {
    recordIndex = D_80146888[D_801E3208].unk_02[4];
    if (D_801E2AB8[recordIndex].unk_1C[1] > 1) {
      cue = 0x20C;
    } else {
      cue = 0x20B;
    }
    func_8015DF18((u16)cue);
    D_1F800044[9] = 4;
    slideWork = D_1F800044;
    slideWork[3]++;
  }
  func_8014D978();
  appendDimTile();
}
