#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x80144908 @kind unknown — six-byte party-id mirror of the live
 * local work records; this callback arms and tears down its first slot's id in
 * addition to the two fixed ids 7 and 4. */
extern u8 D_80144908[];
/* @source 0x80146254 @kind unknown — shared live count of the 0x140-byte-stride
 * local work records, which this callback clears after re-arming the effect
 * group. */
extern u8 D_80146254;

/* @source 0x801FC0C8
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 *           overlay effect group for ids 7, 4 and the party id held in the
 *           first slot of the six-byte party mirror D_80144908, through
 *           func_80166E88(7, 4, D_80144908[0], 0), then clears the shared live
 *           work-record count byte D_80146254, then runs the per-id effect
 *           teardown func_801C187C for id 7, for id 4 and finally for the
 *           mirror's first slot, which it re-reads after those two calls
 *           instead of reusing the id it passed to the arm call, and it
 *           finishes with the no-argument helper func_801C1630. It ignores the
 *           record pointer and the second argument its dispatcher passes, reads
 *           only D_80144908[0] and writes only that D_80146254 byte; the
 *           0x18-byte frame keeps $s0 and $ra because the mirror base is
 *           materialised once into $s0 before the first `lbu` and the third
 *           teardown re-loads through that same saved base, so the two mirror
 *           reads survive the four intervening calls. The zero fourth arm
 *           argument sits in the arm jal's delay slot, each teardown id is
 *           materialised inside its own jal's delay slot and the
 *           func_801C1630 call's delay slot carries the nop. No in-image jal or
 *           j reaches the address; its only image word is 0x801FC4A8, entry 16
 *           of the twenty-two-word overlay callback run at 0x801FC468
 *           (0x801FC468 through 0x801FC4BC) that
 *           dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C
 *           (0x801FB37C) indexes with the unsigned record byte at offset 0x7A,
 *           so the overlay selects this entry for the record kind 0x10. Its
 *           statement order, operand forms and declaration style come from the
 *           exact neighbour
 *           armEffectGroupThenClearWorkRecordsAndResetEffectSlotsThenInvokeHelperScenarioScena1400_801FC074
 *           (0x801FC074), which runs the same arm, clear, three teardowns and
 *           helper tail for the fixed ids 7, 2 and 8; this body differs by
 *           taking its third arm id and third teardown id from the mirror slot
 *           and by the $s0 save that reload needs, which is why it is 0x68
 *           bytes against the neighbour's 0x54. A payload scan of every target
 *           binary found no other copy of this 0x68-byte span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupForMirrorPartyIdThenClearWorkRecordsAndResetEffectSlotsThenInvokeHelperScenarioScena1400_801FC0C8(
    void) {
  func_80166E88(7, 4, D_80144908[0], 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(4);
  func_801C187C(D_80144908[0]);
  func_801C1630();
}
