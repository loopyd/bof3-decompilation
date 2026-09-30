#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FC1E8
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 * overlay effect group for ids 7, 2 and 6 through func_80166E88(7, 2, 6, 0),
 * then clears the shared live-local work-record count byte D_80146254, then
 * runs the per-id effect teardown func_801C187C for id 7 and finishes with the
 * helper func_801C1630. It ignores the record pointer and the second argument
 * its dispatcher passes, reads no memory of its own and writes none; the
 * 0x18-byte frame only keeps $ra across the four calls, the three leading arm
 * immediates are materialised in $a0/$a1/$a2 before the first jal with the zero
 * fourth one in that jal's delay slot, the teardown id is materialised in $a0
 * inside the second jal's delay slot and the func_801C1630 call's delay slot
 * carries the nop. No in-image jal reaches the address; its only image word is
 * 0x801FC4B0, entry 18 of the twenty-two-word overlay callback run at
 * 0x801FC468 (0x801FC468 through 0x801FC4BC) that
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C (0x801FB37C)
 * indexes with the unsigned record byte at offset 0x7A, so the overlay selects
 * this entry for the record kind 0x12. The four-call arm-clear-teardown-tail
 * sequence is the same shape the exact scena08 sibling
 * armEffectGroupThenClearWorkRecordsAndResetEffectSlotScenarioScena0800_801FE698
 * runs for ids 7/2/0x0A with the teardown on 0x0A.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectSlotScenarioScena1400_801FC1E8(void) {
  func_80166E88(7, 2, 6, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C1630();
}
