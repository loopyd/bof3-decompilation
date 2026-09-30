#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

/* @source 0x801FBC88
 * @behavior Overlay per-record callback: it first re-arms the overlay effect
 * group for the ids 7, 5 and 8 through func_80166E88(7, 5, 8, 0), then clears
 * the shared live-local work-record count byte D_80146254 (0x80146254) to zero,
 * and then runs the per-id effect teardown func_801C187C for id 7. It takes no
 * arguments, returns nothing, reads no memory of its own and its only store is
 * that one byte; the body is straight line (one basic block, no branches) and
 * the 0x18-byte frame exists only to keep $ra across the two calls. The three
 * leading argument immediates are materialised in $a0/$a1/$a2 before the first
 * jal and the fourth in that jal's delay slot, the second call's argument
 * immediate occupies its own jal's delay slot, and the teardown call is the
 * same per-id helper the exact siblings clearWorkRecordsAndResetEffectSlots_scena18
 * (0x801F6D04) and clearWorkRecordsAndResetEffectSlots_scena19 (0x801F6D68) use
 * after the same D_80146254 store. The address is entry 6 - the word at
 * 0x801FBFD0, payload offset 0x53D0 - of the thirteen-entry callback-pointer
 * table at 0x801FBFB8 that func_801FB350 (0x801FB350) selects with the record's
 * unsigned byte at offset 0x7A and calls with the record pointer and the
 * scenario flag word D_8014686C, so this handler runs for a record whose 0x7A
 * byte reads 6 and no in-image jal reaches the address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectScenarioScena1300_801FBC88(void) {
  func_80166E88(7, 5, 8, 0);
  D_80146254 = 0;
  func_801C187C(7);
}
