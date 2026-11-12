#include "bof3/bof3.h"

extern u8 D_80146254;
extern u8 D_80144F5D;
extern u8 D_80144F5E;
extern u8 D_80144F5F;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FBFA8
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 * overlay effect group for ids 0, 1 and 5 through func_80166E88(0, 1, 5, 0),
 * then clears the shared live-local work-record count byte D_80146254, then
 * runs the per-id effect teardown func_801C187C for ids 0, 1 and 5 in that
 * order, and finishes with the no-argument helper func_801C1630. After that
 * helper returns it publishes three literal party ids into the upper half of
 * the shared party-id list by storing 7, 8 and 5 into the three consecutive
 * bytes D_80144F5D, D_80144F5E and D_80144F5F, which are slots 3, 4 and 5 of
 * the six-byte list at D_80144F5A. It ignores the record pointer and the
 * second argument its dispatcher passes, reads no memory of its own, writes
 * only those four bytes and returns nothing. The 0x18-byte frame only keeps
 * $ra at 0x10 across the five calls; the three leading arm immediates are
 * materialised in $a0/$a1/$a2 before the frame store, the zero fourth one sits
 * in the first jal delay slot, each teardown id is materialised in $a0 inside
 * its own jal delay slot, the func_801C1630 call delay slot carries the nop,
 * and each of the three tail stores materialises its own constant in $v0 and
 * the 0x8014 page in $at. No in-image jal reaches the address; its only image
 * word is 0x801FC49C, entry 13 of the twenty-two-word overlay callback run at
 * 0x801FC468 (0x801FC468 through 0x801FC4BC) that
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C (0x801FB37C) indexes
 * with the unsigned record byte at offset 0x7A, so the overlay selects this
 * entry for the record kind 0x0D. Its statement order, operand forms and frame
 * come from the exact neighbour
 * armEffectGroupThenClearWorkRecordsAndResetEffectSlotsThenInvokeHelperScenarioScena1400_801FC020
 * (0x801FC020): a payload cmp of the two spans finds the six arm and teardown
 * immediates as the only differing words in the shorter body, so this body is
 * that one shape carrying its own 0, 1 and 5 where the neighbour carries 7, 2
 * and 6, plus the three tail stores at its own end. No extracted payload holds
 * a byte-identical copy of this 120-byte span, and a masked scan of every
 * extracted payload with jal targets ignored finds no other body with these
 * immediates, so it is not a clone of any lifted function. The arm ids 0, 1 and
 * 5 are the row that the sibling
 * armEffectGroupInvalidateModeRecordEntriesClearCountAndResetEffectSlots0And1ScenarioScena0400_801FA0B4
 * (0x801FA0B4, recorded exact) arms before invalidating the same id list.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectSlotsThenInvokeHelperAndPublishPartyIdsScenarioScena1400_801FBFA8(void) {
  func_80166E88(0, 1, 5, 0);
  D_80146254 = 0;
  func_801C187C(0);
  func_801C187C(1);
  func_801C187C(5);
  func_801C1630();
  D_80144F5D = 7;
  D_80144F5E = 8;
  D_80144F5F = 5;
}
