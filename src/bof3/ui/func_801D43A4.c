#include "bof3/ui/shop00_internal.h"

/* @source 0x801D43A4
 * @behavior shop phase sub-step reached through entry 2 of the phase table
 *           D_801E52F0, which the dispatcher func_801D41B0 selects with the UI
 *           sub-step byte D_80148652: appends the fullscreen dim tile through
 *           appendFullscreenDimTile, calls the shared cue dispatcher
 *           func_801636A0 with the constants 0 and 1, re-arms the frame timer
 *           phaseTimer with 0x96 and advances the UI sub-step byte D_80148652
 *           by one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D43A4(void) {
  appendFullscreenDimTile();
  func_801636A0(0, 1);
  phaseTimer = 0x96;
  D_80148652 += 1;
}
