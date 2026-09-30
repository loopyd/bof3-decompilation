#include "bof3/ui/shop00_internal.h"

/* @source 0x801DE88C
 * @behavior sub-step handler reached as entry 0 of the phase table
 *           D_801E5C08 (indexed by the UI sub-step byte D_80148652): when the
 *           global gate halfword D_80143C40 is clear it appends the fullscreen
 *           dim tile, re-arms phaseTimer with 2 and advances the UI sub-step
 *           byte D_80148652 by one; when the gate is set it calls
 *           func_801DF6A8 instead and leaves phaseTimer and D_80148652
 *           untouched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DE88C(void) {
  u8 previous;

  if (D_80143C40 == 0) {
    appendFullscreenDimTileB();
    /* The original reads the sub-step byte before the phaseTimer store and
     * writes it back incremented afterwards (sibling shape: func_801D39F8). */
    previous = D_80148652;
    phaseTimer = 2;
    D_80148652 = previous + 1;
  } else {
    func_801DF6A8();
  }
}
