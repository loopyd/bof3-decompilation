#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
/* 0x801C1ABC — party/character helper; its exact role is unresolved, so it
 * stays address-anchored like the sibling scena09, scena13 and scena14 wrappers
 * that call it. */
void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x80145F09 @kind unknown — party byte (+0x79) of the 0x140-byte-stride
 * local work records based at D_80145E90. */
extern u8 D_80145F09[];
/* @source 0x80144908 @kind unknown — six-byte party-id mirror of the live local
 * work records, whose upper three slots are the parallel list at D_8014490B;
 * this callback writes only its first slot. */
extern u8 D_80144908[];
/* @source 0x80146254 @kind unknown — shared live count of the 0x140-byte-stride
 * local work records, which this callback clears after re-arming the effect
 * group. */
extern u8 D_80146254;

/* @source 0x801FBE10
 * @behavior Overlay per-record callback (entry 11 of the twenty-two-word
 * callback run at 0x801FC468, 0x801FC468 through 0x801FC4BC, which
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C selects with the
 * unsigned record byte at offset 0x7A; the word at 0x801FC494 holds this
 * address, its only image word). It snapshots the party byte of the first three
 * 0x140-byte-stride local work records into an eight-byte stack array and, for
 * every one of those bytes that is neither 7 nor 4, also records it in the
 * first slot of the D_80144908 party mirror, so that slot ends up holding the
 * last qualifying party id the walk visited. In the first loop the counter is a
 * zero-extended u8 bounded by an unsigned compare against 3, the record is
 * addressed as D_80145F09 + counter * 0x140, the two comparison constants stay
 * in $a3 and $a2 and the mirror base in $a1 across the whole loop, and the
 * guard's store keeps the byte it tested while the snapshot reloads the record
 * byte after it. The second loop re-invokes the party helper func_801C1ABC with
 * each of those three snapshot bytes in order, keeping the snapshot base in $s1
 * across the jal and incrementing its counter in that jal's delay slot. It then
 * re-arms the three-entry overlay effect group for ids 10, 0xff and 0xff through
 * func_80166E88(10, 0xff, 0xff, 0), clears the shared live-local work-record
 * count byte D_80146254, runs the per-id effect teardown func_801C187C for id 10
 * and finishes with the helper func_801C1630. It ignores the record pointer and
 * the second argument its dispatcher passes, writes only that mirror slot and
 * D_80146254, and the 0x28-byte frame holds the eight-byte snapshot at $sp+0x10
 * plus the saved $ra at 0x20 and $s0/$s1 at 0x18/0x1C; the arm immediates are
 * materialised in $a0/$a1/$a2 before that jal with the zero fourth one in its
 * delay slot, and the teardown id 10 fills its own jal delay slot. No in-image
 * jal reaches the address. The snapshot-then-helper-then-arm-then-reset order
 * with the func_801C1630 tail is the shape the exact sibling
 * invokePartyHelperForWorkRecordPartyBytesThenArmEffectGroupAndResetEffectScenarioScena1300_801FBDD8
 * (0x801FBDD8) runs from the same 0x28-byte frame, and the mirror write is the
 * one the exact neighbour
 * copyWorkRecordPartyBytesIntoMirrorThenInvokePartyHelperArmEffectGroupAndResetEffectScenarioScena1400_801FC130
 * (0x801FC130) makes per slot; this member differs from both in its two-id skip
 * guard, its single recorded mirror slot and its arm ids 10, 0xff and 0xff.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the record stride is materialised into two separate locals
 * because the original recomputes it at both sites. With one `offset` local
 * shared by the guard and the snapshot the compiler keeps the product live
 * across the guard's store and the body measured 46 of 66 instructions at 252
 * bytes; giving the snapshot its own `step` local, so that each block computes
 * the stride from `i` and both subscript addresses fold into $at, measured 66 of
 * 66 at 264 bytes with no first difference. Spelling the stride inline in the
 * subscript instead makes the compiler hoist the D_80145F09 address into a
 * register, which measured 48 of 66 at the same 264 bytes.
 */
void snapshotWorkRecordPartyBytesAndRecordSelectedPartyInMirrorThenInvokePartyHelperArmEffectGroupAndResetEffectScenarioScena1400_801FBE10(void) {
  u8 slots[8];
  u8 i;
  u32 offset;
  u32 step;

  for (i = 0; i < 3; i++) {
    offset = i * 0x140;
    if (D_80145F09[offset] != 7 && D_80145F09[offset] != 4) {
      D_80144908[0] = D_80145F09[offset];
    }
    step = i * 0x140;
    slots[i] = D_80145F09[step];
  }
  for (i = 0; i < 3; i++) {
    func_801C1ABC(slots[i]);
  }
  func_80166E88(10, 0xff, 0xff, 0);
  D_80146254 = 0;
  func_801C187C(10);
  func_801C1630();
}
