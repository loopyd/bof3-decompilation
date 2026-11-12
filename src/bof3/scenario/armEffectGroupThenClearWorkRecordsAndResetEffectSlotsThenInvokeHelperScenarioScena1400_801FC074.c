#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FC074
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 * overlay effect group for ids 7, 2 and 8 through func_80166E88(7, 2, 8, 0),
 * then clears the shared live-local work-record count byte D_80146254, then
 * runs the per-id effect teardown func_801C187C for ids 7, 2 and 8 in that
 * order, and finishes with the no-argument helper func_801C1630. It ignores
 * the record pointer and the second argument its dispatcher passes, reads no
 * memory of its own and writes only that D_80146254 byte; the 0x18-byte frame
 * only keeps $ra across the five calls, the three leading arm immediates are
 * materialised in $a0/$a1/$a2 before the frame store, the zero fourth one sits
 * in the first jal's delay slot, each teardown id is materialised in $a0 inside
 * its own jal's delay slot and the func_801C1630 call's delay slot carries the
 * nop. No in-image jal or j reaches the address; its only image word is
 * 0x801FC4A4, entry 15 of the twenty-two-word overlay callback run at
 * 0x801FC468 (0x801FC468 through 0x801FC4BC) that
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C (0x801FB37C) indexes
 * with the unsigned record byte at offset 0x7A, so the overlay selects this
 * entry for the record kind 0x0F. Its statement order, operand forms and frame
 * come from the exact neighbour
 * armEffectGroupThenClearWorkRecordsAndResetEffectSlotsThenInvokeHelperScenarioScena1400_801FC020
 * (0x801FC020): a payload cmp of the two 0x54-byte spans reports exactly two
 * differing words, the third arm immediate and the third teardown id, so this
 * body is that one's shape carrying its own 8 where the neighbour carries 6.
 * The 0x54-byte span is byte-identical to the exact scena08 body at
 * 0x801FE7AC, which arms and tears down the same ids 7, 2 and 8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectSlotsThenInvokeHelperScenarioScena1400_801FC074(void) {
  func_80166E88(7, 2, 8, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(2);
  func_801C187C(8);
  func_801C1630();
}
