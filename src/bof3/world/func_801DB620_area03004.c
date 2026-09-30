#include "bof3/world/area03004_internal.h"

/* @source 0x801DB620
 * @behavior AREA030 panel step: decrements byte 9 of the work record published
 * at the scratchpad cursor 0x1F800044 and submits the panel pair at y 78 with
 * the -40-per-count vertical offset; once that count reaches zero it picks the
 * frontend cue 0x20C when the D_801E2AB8 scale-record byte +0x1D selected by
 * byte 6 of the active 0x98-byte work record D_80146888[D_801E3208] reads above
 * 1 and the cue 0x20B otherwise, dispatches it through func_8015DF18, restarts
 * the countdown at 4 and advances scratch byte 3; finally it always runs
 * func_8014D978 and appends the dim tile through appendDimTile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DB620(void) {
  u8* work;
  u32 slot;
  u32 cue;

  work = D_1F800044;
  work[9] = work[9] - 1;
  submitPanelPair((s16)(-40 * D_1F800044[9]), 78);
  if (D_1F800044[9] == 0) {
    slot = D_80146888[D_801E3208].unk_02[4];
    if (D_801E2AB8[slot].unk_1C[1] > 1) {
      cue = 0x20C;
    } else {
      cue = 0x20B;
    }
    func_8015DF18((u16)cue);
    D_1F800044[9] = 4;
    work = D_1F800044;
    work[3]++;
  }
  func_8014D978();
  appendDimTile();
}
