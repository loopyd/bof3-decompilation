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
 * this callback fills its first three slots. */
extern u8 D_80144908[];
/* @source 0x80146254 @kind unknown — shared live count of the 0x140-byte-stride
 * local work records, which this callback clears after re-arming the effect
 * group. */
extern u8 D_80146254;

/* @source 0x801FC130
 * @behavior Overlay per-record callback: it copies the party byte of the first
 *           three 0x140-byte-stride local work records into the party mirror,
 *           re-invokes the party helper for those three mirrored ids, and then
 *           re-arms the overlay effect group and resets one effect slot. The
 *           first loop walks the records based at D_80145E90 with a
 *           zero-extended u8 counter bounded by an unsigned compare against 3
 *           and stores each record's party byte (+0x79, at D_80145F09) into
 *           D_80144908 at the same index, so the mirror receives the three ids
 *           with no stack snapshot of its own. The second loop over the same
 *           bound then calls func_801C1ABC with the mirror's first three bytes
 *           in order, with no id test. It then re-arms the overlay effect group
 *           for the ids 7, 8 and 4 through func_80166E88(7, 8, 4, 0), clears
 *           the shared live work-record count byte D_80146254 to zero, runs the
 *           per-id effect teardown func_801C187C for id 7 and finishes with the
 *           helper func_801C1630. It ignores the record pointer and the
 *           scenario flag word its dispatcher passes, reads only those record
 *           party bytes, and writes nothing but the three mirror slots and
 *           D_80146254. The 0x20-byte frame reserves eight bytes at $sp+0x10
 *           that no emitted instruction touches and saves only $s0 at 0x18 and
 *           $ra at 0x1C - the counter lives in $s0 and the zero-extended index
 *           in $v0 across both loops, so no second saved register is needed,
 *           each loop bounds its counter with `andi` plus `sltiu` against 3,
 *           the three arm immediates are materialised in $a0/$a1/$a2 before
 *           that jal with the zero fourth one in its delay
 *           slot, the teardown id 7 fills its own jal delay slot, and the final
 *           call keeps a plain nop slot. The address is entry 17 - the word at
 *           0x801FC4AC, payload offset 0x58AC - of the twenty-two-entry
 *           callback-pointer run at 0x801FC468 (0x801FC468 through 0x801FC4BC)
 *           that dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C
 *           selects with the record's unsigned byte at offset 0x7A, so this
 *           handler runs for a record whose 0x7A byte reads 0x11; that word is
 *           its only image word and no in-image jal reaches the address. The
 *           copy-then-helper-then-arm-then-reset order with the func_801C1630
 *           tail is the shape the exact sibling
 *           invokePartyHelperForWorkRecordPartyBytesThenArmEffectGroupAndResetEffectScenarioScena1300_801FBDD8
 *           (0x801FBDD8) runs from a stack snapshot, differing in that sibling's
 *           arm ids 7, 4, 2 and its 0x28-byte frame, and from the exact
 *           neighbour armEffectGroupThenClearWorkRecordsAndResetEffectSlotScenarioScena1400_801FC1E8
 *           (0x801FC1E8, 0x801FC4B0) which arms 7, 2, 6 with no mirror pass.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: two things set this body's frame. The scaled stride sits in a
 * separate `offset` local, the form the exact sibling
 * copyRecordPartyBytesThenInvokeHelperScenarioScena0900_801FE234 records for the
 * same D_80145F09 stride: with the product inline in that subscript gcc hoists
 * the symbol address into a saved register instead of folding it into $at per
 * iteration. The unused `slots` declaration is an orphan stack slot: the
 * original's 0x20-byte frame reserves eight bytes at $sp+0x10 that no emitted
 * instruction touches, while this body's own locals need only $s0 and $ra.
 * Measured without it, 40 of 46 instructions with the entry `addiu sp,sp,-24`
 * against the original's -32 and the two save offsets and both restores
 * differing with it; with the eight-byte declaration the sweep measured 46 of
 * 46 at 184 bytes with no first difference. The bytes constrain only that size,
 * so the name and type are a reconstruction of the scratch region the exact
 * sibling invokePartyHelperForWorkRecordPartyBytesThenArmEffectGroupAndResetEffectScenarioScena1300_801FBDD8
 * gives the same func_801C1ABC calls, and removing it leaves the frame and save
 * offsets as the only residual.
 */
void copyWorkRecordPartyBytesIntoMirrorThenInvokePartyHelperArmEffectGroupAndResetEffectScenarioScena1400_801FC130(
    void) {
  u8 slots[8];
  u8 i;
  u32 offset;

  for (i = 0; i < 3; i++) {
    offset = i * 0x140;
    D_80144908[i] = D_80145F09[offset];
  }
  for (i = 0; i < 3; i++) {
    func_801C1ABC(D_80144908[i]);
  }
  func_80166E88(7, 8, 4, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C1630();
}
