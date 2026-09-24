#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

/* @source 0x801F6D68
 * @behavior Re-arms this overlay's three-entry effect group with
 * @status exact
 * @match 100.00
 * @residual none
 * func_80166E88(0, 3, 4, 0), clears the shared live-local-work-record count byte
 * D_80146254 (0x80146254) to zero, and then runs the per-id effect teardown
 * func_801C187C for ids 0, 3 and 4 in that order; the body is straight line (one
 * basic block, no branches) and touches no memory other than that one byte store.
 * It is the scena19 twin of scena18's clearWorkRecordsAndResetEffectSlots_scena18
 * (0x801F6D04) with the same five calls and the same store, and per this overlay's
 * dispatcher doc (dispatchProgressHandler_scena19) it is entry 5 of the overlay's
 * seven-entry per-frame progress handler table at 0x801F6DD0, selected while the
 * shared signed scenario-progress byte D_80146872 reads 5. It takes no arguments,
 * returns nothing, and reads no game state.
 */
void clearWorkRecordsAndResetEffectSlots_scena19(void) {
  func_80166E88(0, 3, 4, 0);
  D_80146254 = 0;
  func_801C187C(0);
  func_801C187C(3);
  func_801C187C(4);
}
