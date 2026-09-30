#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

/* @source 0x801FBDC4
 * @behavior Overlay per-record callback (entry 10 of the twenty-two-word
 * callback run at 0x801FC468, 0x801FC468 through 0x801FC4BC, which
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C selects with the
 * unsigned record byte at offset 0x7A; the word at 0x801FC490 holds this
 * address, its only image word). It re-arms the three-entry overlay effect
 * group for ids 7, 5 and 6 through func_80166E88(7, 5, 6, 0), clears the
 * shared live-local work-record count byte D_80146254, then runs the per-id
 * effect teardown func_801C187C for ids 7, 5 and 6 in that order and returns
 * without the func_801C1630 tail its neighbour 0x801FBF18 carries. It ignores
 * the record pointer and the second argument its dispatcher passes, reads no
 * memory of its own and writes only that D_80146254 byte; the 0x18-byte frame
 * only keeps $ra across the four calls, the three leading arm immediates are
 * materialised in $a0/$a1/$a2 before the first jal with the zero fourth one in
 * that jal's delay slot, and each teardown id is materialised in $a0 inside
 * its own jal's delay slot. No in-image jal reaches the address. Its bytes are
 * instruction-for-instruction identical to its neighbour 0x801FBD78 apart from
 * the second arm and teardown immediate (5 where that one has 4).
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectSlotsScenarioScena1400_801FBDC4(void) {
  func_80166E88(7, 5, 6, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(5);
  func_801C187C(6);
}
