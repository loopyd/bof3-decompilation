#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

/* @source 0x801FBD2C
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 * overlay effect group for ids 7, 8 and 4 through func_80166E88(7, 8, 4, 0),
 * then clears the shared live-local work-record count byte D_80146254, then
 * runs the per-id effect teardown func_801C187C for ids 7, 4 and 8 in that
 * order, and returns without the func_801C1630 tail its neighbour 0x801FBF18
 * in the same run carries. It ignores the record pointer and the second
 * argument its dispatcher passes, reads no memory of its own and writes none;
 * the 0x18-byte frame only keeps $ra across the four calls, the three leading
 * arm immediates are materialised in $a0/$a1/$a2 before the first jal with the
 * zero fourth one in that jal's delay slot, and each teardown id is
 * materialised in $a0 inside its own jal's delay slot. No in-image jal reaches
 * the address; its only image word is 0x801FC488, entry 8 of the
 * twenty-two-word overlay callback run at 0x801FC468 (0x801FC468 through
 * 0x801FC4BC) that dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C
 * (0x801FB37C) indexes with the unsigned record byte at offset 0x7A, so the
 * overlay selects this entry for the record kind 8. Its immediate neighbours
 * 0x801FBD78 and 0x801FBDC4 are the same arm-clear-teardown shape for the id
 * triples 7/4/6 and 7/5/6.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectSlotsScenarioScena1400_801FBD2C(void) {
  func_80166E88(7, 8, 4, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(4);
  func_801C187C(8);
}
