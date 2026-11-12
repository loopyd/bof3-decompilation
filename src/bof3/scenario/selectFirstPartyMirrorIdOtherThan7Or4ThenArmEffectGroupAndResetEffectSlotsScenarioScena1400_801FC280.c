#include "bof3/bof3.h"

/* Six-byte party-id mirror of the live local work records. */
extern u8 D_80144908[];
/* Shared live count of the 0x140-byte-stride local work records. */
extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FC280
 * @behavior Overlay per-record callback: it scans the first three slots of the
 * six-byte party-id mirror D_80144908 and stops at the first byte that is
 * neither 7 nor 4, so the scan ends either on that byte or, when every visited
 * slot holds 7 or 4, on the third slot's value. With that id it re-arms the
 * three-entry overlay effect group for ids 7, 4 and the selected id through
 * func_80166E88(7, 4, id, 0), clears the shared live-local work-record count
 * byte D_80146254, runs the per-id effect teardown func_801C187C for ids 7, 4
 * and the selected id in that order, and finishes with the no-argument helper
 * func_801C1630. It ignores the record pointer and the second argument its
 * dispatcher passes, reads only D_80144908 and writes only that D_80146254
 * byte. The 0x18-byte frame keeps $s0 and $ra because the selected id stays in
 * $s0 across the calls while $v0 carries the masked loop index; the two
 * comparison constants 7 and 4 sit in $a1 and $a0 across the whole loop and
 * are re-materialised for the arm call, the zero fourth arm argument sits in
 * that jal's delay slot, each teardown id is materialised inside its own jal's
 * delay slot and the func_801C1630 call's delay slot carries the nop. No
 * in-image jal reaches the address; its only image word is 0x801FC4B8, entry
 * 20 of the twenty-two-word overlay callback run at 0x801FC468 (0x801FC468
 * through 0x801FC4BC) that
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C (0x801FB37C)
 * indexes with the unsigned record byte at offset 0x7A, so the overlay selects
 * this entry for the record kind 0x14. Its statement order, operand forms and
 * declaration style come from the exact scena09 sibling
 * clearPartySlotsThenRearmEffectGroupAndResetEffectSlotsScenarioScena0900_801FDF54
 * (0x801FDF54), which runs the same mirror scan, arm, clear, three teardowns
 * and helper tail; this body only reads the mirror, leaves every visited slot
 * unchanged, breaks instead of continuing once a slot qualifies and carries
 * its own 4 where that sibling carries 2. A scan of every extracted target
 * payload found no other copy of this 0xA8-byte span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectFirstPartyMirrorIdOtherThan7Or4ThenArmEffectGroupAndResetEffectSlotsScenarioScena1400_801FC280(
    void) {
  u8 slot;
  u8 i;

  i = 0;
  for (; i < 3; i++) {
    slot = D_80144908[i];
    if (slot != 7 && slot != 4) {
      break;
    }
  }
  func_80166E88(7, 4, slot, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(4);
  func_801C187C(slot);
  func_801C1630();
}
