#include "bof3/battle/battle03_internal.h"

/* @source 0x801E15CC
 * @behavior Runs the first local readiness helper, then selects the scratch
 * work state from the published local work record halfword +0x80: with bit
 * 0x4000 set it stores 6 then 4 into scratch work bytes +0x01/+0x02, otherwise
 * it clears the pending bit selected by scratch work byte +0x05 and stores 3
 * then 0 into those bytes; both paths then clear scratch work byte +0x03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E15CC(void) {
  localReadyOrHelper1();
  if (D_80146250->unk_80 & 0x4000) {
    D_1F800044->unk_01 = 6;
    D_1F800044->unk_02 = 4;
  } else {
    clearPendingBit(D_1F800044->unk_05);
    D_1F800044->unk_01 = 3;
    D_1F800044->unk_02 = 0;
  }
  D_1F800044->unk_03 = 0;
}
