#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x80146254 @kind unknown — shared live count of the local work
 * records, which this callback clears after re-arming the effect group. */
extern u8 D_80146254;

/* @source 0x801FBD84
 * @behavior Overlay per-record callback: it first re-arms the overlay effect
 *           group for the ids 7, 5 and 8 through func_80166E88(7, 5, 8, 0),
 *           then clears the shared live-local work-record count byte
 *           D_80146254 to zero, then runs the per-id effect teardown
 *           func_801C187C for ids 7, 5 and 8 in that order, and finishes with
 *           the helper func_801C1630. It takes no arguments, returns nothing,
 *           reads no memory of its own and its only store is that one byte;
 *           the body is straight line, one basic block with no branches, and
 *           the 0x18-byte frame exists only to keep $ra across the five calls.
 *           The three leading arm immediates are materialised in
 *           $a0/$a1/$a2 before the first jal and the zero fourth one in that
 *           jal's delay slot, each teardown id is materialised in $a0 inside
 *           its own jal's delay slot, and the final call keeps a plain nop
 *           slot. The address is entry 8 - the word at 0x801FBFD8, payload
 *           offset 0x53D8 - of the thirteen-entry callback-pointer table at
 *           0x801FBFB8 that dispatchRecordCallbackScenarioScena1300_801FB350
 *           selects with the record's unsigned byte at offset 0x7A and calls
 *           with the record pointer and the scenario flag word D_8014686C, so
 *           this handler runs for a record whose 0x7A byte reads 8, and no
 *           in-image jal reaches the address. Its arm-then-reset-then-per-id
 *           teardown order is that of the exact neighbour
 *           armEffectGroupThenClearWorkRecordsAndResetEffectSlotsScenarioScena1300_801FBB78
 *           (0x801FBB78), which passes the arm ids 7, 5 and 6 and tears down
 *           7, 6 and 5 without a trailing helper, and its arm ids 7, 5 and 8
 *           are those of the exact neighbour
 *           armEffectGroupThenClearWorkRecordsAndResetEffectScenarioScena1300_801FBC88
 *           (0x801FBC88), which tears down only id 7; the func_801C1630 tail
 *           is the call invokeHelperArgThenHelperScenarioScena1300_801FBE98
 *           (0x801FBE98) finishes with.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectSlotsThenInvokeHelperScenarioScena1300_801FBD84(
    void) {
  func_80166E88(7, 5, 8, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(5);
  func_801C187C(8);
  func_801C1630();
}
