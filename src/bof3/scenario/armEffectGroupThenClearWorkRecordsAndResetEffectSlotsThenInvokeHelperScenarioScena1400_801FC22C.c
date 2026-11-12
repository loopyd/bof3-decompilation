#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FC22C
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 * overlay effect group for ids 7, 8 and 4 through func_80166E88(7, 8, 4, 0),
 * then clears the shared live-local work-record count byte D_80146254, then
 * runs the per-id effect teardown func_801C187C for ids 7, 8 and 4 in that
 * order, and finishes with the no-argument helper func_801C1630. It ignores
 * the record pointer and the second argument its dispatcher passes, reads no
 * memory of its own and writes only that D_80146254 byte; the 0x18-byte frame
 * only keeps $ra across the five calls, the three leading arm immediates are
 * materialised in $a0/$a1/$a2 before the frame store, the zero fourth one
 * sits in the first jal's delay slot, each teardown id is materialised in $a0
 * inside its own jal's delay slot and the func_801C1630 call's delay slot
 * carries the nop. No in-image jal reaches the address; its only image word is
 * 0x801FC4B4, entry 19 of the twenty-two-word overlay callback run at
 * 0x801FC468 (0x801FC468 through 0x801FC4BC) that
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C (0x801FB37C)
 * indexes with the unsigned record byte at offset 0x7A, so the overlay selects
 * this entry for the record kind 0x13. Its statement order, operand forms and
 * frame come from the exact neighbour
 * armEffectGroupThenClearWorkRecordsAndResetEffectSlotsScenarioScena1400_801FBD2C
 * (0x801FBD2C), whose bytes are this body's first 0x4C bytes with the second
 * and third teardown ids exchanged and this body's func_801C1630 tail absent,
 * and the tail from the exact neighbour
 * armEffectGroupThenClearWorkRecordsAndResetEffectSlotScenarioScena1400_801FC1E8
 * (0x801FC1E8). The 0x54-byte span is byte-identical (one payload cmp) to the
 * still-unlifted func_801FE520 in the scena15/00 payload at file offset 0x7920.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectSlotsThenInvokeHelperScenarioScena1400_801FC22C(void) {
  func_80166E88(7, 8, 4, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(8);
  func_801C187C(4);
  func_801C1630();
}
