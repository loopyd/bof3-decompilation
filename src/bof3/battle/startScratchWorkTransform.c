#include "bof3/battle/battle03_internal.h"

/* @source 0x801E04B0
 * @behavior Starts the scratch work position transform selected by the published
 * local work record 0x80146250: when that record's halfword +0x80 carries bit
 * 0x4000 it stores 2 into scratch work byte +0x03 and clears the +0x0A countdown
 * byte; otherwise it derives the -0x2000 delta, clears record halfword +0x11C,
 * publishes the scratch work delta pair +0x0C/+0x10 as 0/-0x2000, mode-transforms
 * that pair, reloads the +0x0A countdown byte to 4, runs localReadyOrHelper2 and
 * func_8015D71C(0x205), then advances scratch work byte +0x03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startScratchWorkTransform(void) {
  Battle03LocalWork* record;
  Battle03LocalWork* work;

  record = D_80146250;
  if ((record->unk_80 & 0x4000u) != 0u) {
    D_1F800044->unk_03 = 2u;
    D_1F800044->pad_09[1] = 0u;
  } else {
    s32 delta = -0x2000;
    work = D_1F800044;
    record->unk_11c = 0;
    work->unk_0c = 0;
    work->unk_10 = delta;
    transformFirstPointPairByMode((s32)work);
    D_1F800044->pad_09[1] = 4;
    localReadyOrHelper2();
    func_8015D71C(0x205);
    D_1F800044->unk_03++;
  }
}
