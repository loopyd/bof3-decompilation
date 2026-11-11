#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

/* @source 0x80146254 @kind unknown — shared live count of the local work
 * records, which this callback clears after re-arming the effect group. */
extern u8 D_80146254;
/* @source 0x80144908 @kind unknown — six-byte party-id mirror of the live local
 * work records; this callback reads all six slots of it. */
extern u8 D_80144908[];
/* @source 0x80144F5A @kind unknown — shared six-byte party-id list of the live
 * work records, whose upper three slots are the parallel list at D_80144F5D. */
extern u8 D_80144F5A[];

/* @source 0x801FBCC4
 * @behavior Overlay per-record callback: it snapshots the first three slots of
 *           the D_80144908 party mirror into a stack array, re-arms the overlay
 *           effect group through func_80166E88 with those three snapshot ids
 *           (the fourth argument zeroed in that jal's delay slot), clears the
 *           shared live work-record count byte D_80146254, then runs the
 *           per-id helper func_801C187C for the three snapshot ids in order and
 *           finally publishes the mirror's slots 3..5 into the upper half of
 *           the shared party-id list D_80144F5A. It ignores the record pointer
 *           and the second argument its dispatcher passes, reads no state
 *           besides that mirror, writes only D_80146254 and those three list
 *           bytes, and returns nothing. The 0x20-byte frame holds the 8-byte
 *           snapshot at $sp+0x10 and $ra at 0x18; the snapshot loop and the
 *           publishing loop each bound their zero-extended u8 counter with an
 *           unsigned compare against 3 and 6, the slots base is materialised
 *           once in $a1 before the first loop, and the
 *           D_80146254 store is scheduled into the slot before the first
 *           func_801C187C jal. The address is entry 7 - the word at 0x801FBFD4,
 *           payload offset 0x53D4 - of the thirteen-entry callback-pointer
 *           table at 0x801FBFB8 that the dispatcher
 *           dispatchRecordCallbackScenarioScena1300_801FB350 selects with the
 *           record's unsigned byte at offset 0x7A and calls with the record
 *           pointer and the scenario flag word D_8014686C, so this handler runs
 *           for a record whose 0x7A byte reads 7 and no in-image jal reaches the
 *           address. The arm-then-reset-then-per-id-helper order is the shape of
 *           the neighbours 0x801FBB78 and 0x801FBC88, which pass literal ids and
 *           have no snapshot or publishing loop.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupFromPartyMirrorThenResetWorkCountAndPublishPartyListScenarioScena1300_801FBCC4(
    void) {
  u8 slots[8];
  u8 i;

  for (i = 0; i < 3; i++) {
    slots[i] = D_80144908[i];
  }
  func_80166E88(slots[0], slots[1], slots[2], 0);
  D_80146254 = 0;
  func_801C187C(slots[0]);
  func_801C187C(slots[1]);
  func_801C187C(slots[2]);
  for (i = 3; i < 6; i++) {
    D_80144F5A[i] = D_80144908[i];
  }
}
