#include "bof3/battle/battle03_internal.h"

/* @source 0x801E4A50
 * @behavior Selects the scratch work state from the current enemy work flag
 * 0x4000: with the flag set it stores 6 then 4 into work bytes +0x01/+0x02,
 * otherwise it clears the pending bit selected by work byte +0x05 and stores 2
 * then 0 into those bytes; both paths then clear work byte +0x03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E4A50(void) {
  if ((battleCurrentEnemyWorkState->unk_82 & 0x4000u) != 0u) {
    D_1F800044->unk_01 = 6;
    D_1F800044->unk_02 = 4;
  } else {
    clearPendingBit(D_1F800044->unk_05);
    D_1F800044->unk_01 = 2;
    D_1F800044->unk_02 = 0;
  }
  D_1F800044->unk_03 = 0;
}
