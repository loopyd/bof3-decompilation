#include "bof3/bof3.h"

extern u8 *boss028CurrentRecordState; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay image; this
 * target's generated undefined_syms_auto.txt already binds the address. This
 * body only samples that record's unsigned byte 0, the flag byte whose bit 0
 * gates the step advance. The exact sibling
 * clearWorkByte24Byte2AAndMode48SetByte29To5ApplyLookup15CRequestResource6Copy
 * BattleRecordPositionSeedDistanceThenAdvanceStepBossBoss02816_800C2024 reads
 * the same cell for the position pair this overlay passes to the distance
 * helper. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */


/* Shared main-executable per-frame update helper, bound by this target's
 * generated undefined_funcs_auto.txt row. It is not part of any payload here,
 * so its body stays outside this lift; this caller passes no arguments, as the
 * exact sibling invokeHelperOverlay2 declares for its own
 * callee. */
void func_8014D978(void);

/* @source 0x800C20C8
 * @behavior Entry 1 of this overlay's three-word step run based at 0x800C21BC,
 *           whose words are 0x800C2024, 0x800C20C8 and 0x800C2120; the exact
 *           sibling dispatchWorkByte2Table21bcThenInvokeSharedServiceWhenWork
 *           Byte0Bit0SetBossBoss02816_800C1FB8 indexes that base with the work
 *           record byte 2, and the exact sibling
 *           clearWorkByte24Byte2AAndMode48SetByte29To5ApplyLookup15CRequest
 *           Resource6CopyBattleRecordPositionSeedDistanceThenAdvanceStep
 *           BossBoss02816_800C2024 ends by advancing that byte to hand the run
 *           to this entry. No jal in the payload names this span, so its
 *           prologue and return path place the boundary.
 *           It samples unsigned byte 0 of the shared battle record the main-RAM
 *           cell 0x801EB4E0 points at and tests bit 0 of that byte. While the
 *           bit is clear it advances the work record's step byte 2 by one at
 *           byte width through the scratchpad pointer cell 0x1F800044, which
 *           moves the dispatcher to entry 2 (0x800C2120); while the bit is set
 *           it leaves the step where it is. Either way it then runs the shared
 *           main-executable per-frame update func_8014D978 with no arguments.
 *           The roles behind battle record byte 0 bit 0 and work byte 2 stay
 *           offset-anchored: this lift establishes the test and the counter
 *           bump, not what the flag selects.
 *           The 0x18-byte frame keeps $ra at 0x10 across the one call and the
 *           body takes no arguments; the value the update leaves in $v0 is
 *           returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateRunSharedUpdate(
    void) {
  if ((D_801EB4E0[0] & 1) == 0) {
    boss028CurrentRecordState[2]++;
  }
  func_8014D978();
}
