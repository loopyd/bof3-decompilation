#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

/* @source 0x801FBB78
 * @behavior Overlay per-record callback: it first re-arms the overlay effect
 * group for the ids 7, 5 and 6 through func_80166E88(7, 5, 6, 0), then clears
 * the shared live-local work-record count byte D_80146254 (0x80146254) to zero,
 * and then runs the per-id effect teardown func_801C187C for ids 7, 6 and 5 in
 * that order. It takes no arguments, returns nothing and reads no memory of its
 * own; its only store is that one byte. The body is straight line, one basic
 * block with no branches, and the 0x18-byte frame exists only to keep $ra across
 * the four calls. The three leading arm immediates are materialised in
 * $a0/$a1/$a2 before the first jal and the fourth one in that jal's delay slot,
 * and each teardown id is materialised in $a0 inside its own jal's delay slot.
 * The address is entry 4 - the word at 0x801FBFC8, payload offset 0x53C8 - of
 * the thirteen-entry callback-pointer table at 0x801FBFB8 that the exact
 * dispatcher dispatchRecordCallbackScenarioScena1300_801FB350 (0x801FB350)
 * selects with the record's unsigned byte at offset 0x7A and calls with the
 * record pointer and the scenario flag word D_8014686C, so this handler runs
 * for a record whose 0x7A byte reads 4, and no in-image jal reaches the address.
 * Its 76 payload bytes carry the same arm-then-teardown shape as the exact
 * sibling armEffectGroupThenClearWorkRecordsAndResetEffectSlotsScenarioScena1400_801FBDC4
 * (0x801FBDC4), whose last two teardown ids are 5 and 6 where this span has 6
 * and 5, and as this target's neighbour
 * armEffectGroupThenClearWorkRecordsAndResetEffectScenarioScena1300_801FBC88
 * (0x801FBC88), which tears down the single id 7.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectSlotsScenarioScena1300_801FBB78(void) {
  func_80166E88(7, 5, 6, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(6);
  func_801C187C(5);
}
