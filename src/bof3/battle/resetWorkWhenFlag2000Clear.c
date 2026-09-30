#include "bof3/battle/battle03_internal.h"

/* @source 0x801E13B8
 * @behavior when the current work record flag 0x2000 is clear, clears the
 * record 0x70 and 0x200 bits, clears the scratch pending bit and resets the
 * scratch work state to 2 with bytes 2, 3 and 4 cleared.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkWhenFlag2000Clear(void) {
  Battle03LocalWork* work;

  work = D_80146250;
  if ((work->unk_124 & 0x2000u) != 0u) {
    return;
  }
  work->unk_124 &= ~0x70u;
  work->unk_124 &= ~0x200u;
  clearPendingBit(D_1F800044->unk_05);
  D_1F800044->unk_01 = 2;
  D_1F800044->unk_02 = 0;
  D_1F800044->unk_03 = 0;
  D_1F800044->unk_04 = 0;
}
