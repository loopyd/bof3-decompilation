#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801FE800(void);

/* @source 0x801FE840
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 * overlay effect group for ids 7, 8 and 4 through func_80166E88(7, 8, 4, 0) and
 * then runs the sibling reset at 0x801FE800, which clears the shared live-local
 * work-record count byte D_80146254, runs the per-id teardown func_801C187C for
 * ids 7, 8 and 4 in that order and finishes with func_801C1630(). It ignores
 * the record pointer and second argument its caller passes, reads no memory of
 * its own and writes none; the 0x18-byte frame only keeps $ra across the two
 * calls. The address is entry 9 (the word reading 0x801FE840 at 0x801FE9E4) of
 * the eleven-entry callback table at 0x801FE9C0 that
 * dispatchRecordCallbackScenarioScena0800_801FE018 selects from the record's
 * byte at offset 0x7A, and it is the immediate neighbour of func_801FE800,
 * which is entry 8 and which tears down the same three effect ids this function
 * arms.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenResetEffectSlotsScenarioScena0800_801FE840(void) {
  func_80166E88(7, 8, 4, 0);
  func_801FE800();
}
