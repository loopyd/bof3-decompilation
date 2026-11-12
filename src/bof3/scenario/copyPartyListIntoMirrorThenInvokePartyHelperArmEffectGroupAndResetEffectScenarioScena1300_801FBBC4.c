#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
/* 0x801C1ABC — party/character helper; its exact role is unresolved, so it
 * stays address-anchored like the sibling scena09 and scena14 wrappers that
 * call it. */
void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x80144908 @kind unknown — six-byte party-id mirror of the live local
 * work records, whose upper three slots are the parallel list at D_8014490B;
 * this callback fills all six slots. */
extern u8 D_80144908[];
/* @source 0x80144F5A @kind unknown — shared six-byte party-id list of the live
 * work records, whose upper three slots are the parallel list at D_80144F5D;
 * this callback reads all six slots of it. */
extern u8 D_80144F5A[];
/* @source 0x80146254 @kind unknown — shared live count of the local work
 * records, which this callback clears after re-arming the effect group. */
extern u8 D_80146254;

/* @source 0x801FBBC4
 * @behavior Overlay per-record callback: it copies the shared party-id list
 *           into the party mirror, re-invokes the party helper for the three
 *           mirrored ids, and then re-arms the overlay effect group and resets
 *           one effect slot. One loop with a zero-extended u8 counter bounded
 *           by an unsigned compare against 3 copies the list's slots 0..2
 *           (D_80144F5A) into the mirror's slots 0..2 (D_80144908) and its
 *           slots 3..5 (D_80144F5D) into the mirror's slots 3..5 (D_8014490B),
 *           each iteration loading both source bytes before storing either and
 *           materialising the 0x8014 page in $at on its own for all four
 *           accesses; a second loop over the same bound then calls
 *           func_801C1ABC with the mirror's first three bytes in order. It then
 *           re-arms the overlay effect group for the ids 7, 4 and 8 through
 *           func_80166E88(7, 4, 8, 0), clears the shared live work-record count
 *           byte D_80146254 to zero, runs the per-id effect teardown
 *           func_801C187C for id 7 and finishes with the helper func_801C1630.
 *           It ignores the record pointer and the scenario flag word its
 *           dispatcher passes, reads only that list, and writes nothing but
 *           those six mirror bytes and D_80146254. The 0x18-byte frame holds
 *           only $ra at 0x14 and $s0 at 0x10 - the counter lives in $s0 and the
 *           zero-extended index in $v0 across both loops, and no stack snapshot
 *           exists because the first loop stores straight into the mirror. The
 *           three arm immediates are materialised in $a0/$a1/$a2 before that
 *           jal with the zero fourth one in its delay slot, the teardown id 7
 *           fills its own jal delay slot, and the final call keeps a plain nop
 *           slot. The address is entry 5 - the word at 0x801FBFCC, payload
 *           offset 0x53CC - of the thirteen-entry callback-pointer table at
 *           0x801FBFB8 that dispatchRecordCallbackScenarioScena1300_801FB350
 *           selects with the record's unsigned byte at offset 0x7A and calls
 *           with the record pointer and D_8014686C, so this handler runs for a
 *           record whose 0x7A byte reads 5 and no in-image jal reaches the
 *           address. The copy-then-helper-then-arm-then-reset order with the
 *           func_801C1630 tail is the shape the exact sibling
 *           invokePartyHelperForWorkRecordPartyBytesThenArmEffectGroupAndResetEffectScenarioScena1300_801FBDD8
 *           (0x801FBDD8) runs from a per-record snapshot, differing only in that
 *           sibling's arm ids 7, 4, 2, its 0x140-byte record stride and its
 *           stack array, where this span copies the six list bytes into the
 *           mirror from the same two loops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyPartyListIntoMirrorThenInvokePartyHelperArmEffectGroupAndResetEffectScenarioScena1300_801FBBC4(
    void) {
  u8 i;

  for (i = 0; i < 3; i++) {
    D_80144908[i] = D_80144F5A[i];
    D_80144908[i + 3] = D_80144F5A[i + 3];
  }
  for (i = 0; i < 3; i++) {
    func_801C1ABC(D_80144908[i]);
  }
  func_80166E88(7, 4, 8, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C1630();
}
