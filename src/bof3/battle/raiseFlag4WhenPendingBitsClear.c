#include "bof3/battle/battle15_internal.h"

/* @source 0x800A84FC
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 * and invokes the shared helper at 0x801E5988 while the shared halfword at
 * 0x801463C2 (the pending-battler bitset the sibling lifts set and clear) is
 * zero. Second entry of the two-handler table selected by scratchpad work byte
 * 0x01 in dispatchWorkByte1Pair.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4WhenPendingBitsClear(void) {
  if (D_801463C2 == 0) {
    D_801462E8 |= 4;
    func_801E5988();
  }
}
