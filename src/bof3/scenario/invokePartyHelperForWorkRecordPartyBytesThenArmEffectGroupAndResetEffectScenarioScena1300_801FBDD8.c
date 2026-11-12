#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
/* 0x801C1ABC — party/character helper; its exact role is unresolved, so it
 * stays address-anchored like the sibling scena09 and scena14 wrappers that
 * call it. */
void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x80145F09 @kind unknown — party byte (+0x79) of the 0x140-byte-stride
 * local work records based at 0x80145E90. */
extern u8 D_80145F09[];
/* @source 0x80146254 @kind unknown — shared live count of the 0x140-byte-stride
 * local work records, which this callback clears after re-arming the effect
 * group. */
extern u8 D_80146254;

/* @source 0x801FBDD8
 * @behavior Overlay per-record callback: it snapshots the party byte of the
 *           first three 0x140-byte-stride local work records, re-invokes the
 *           party helper for each snapshot slot, and then re-arms the overlay
 *           effect group and resets one effect slot. It walks the records based
 *           at D_80145E90 with a zero-extended u8 counter bounded by an
 *           unsigned compare against 3, copies each record's party byte
 *           (+0x79, at D_80145F09) into an eight-byte stack array, and then
 *           calls func_801C1ABC with the three snapshot bytes in order. It then
 *           re-arms the overlay effect group for the ids 7, 4 and 2 through
 *           func_80166E88(7, 4, 2, 0), clears the shared live work-record count
 *           byte D_80146254 to zero, runs the per-id effect teardown
 *           func_801C187C for id 7 and finishes with the helper func_801C1630.
 *           It ignores the record pointer and the scenario flag word its
 *           dispatcher passes, reads only those record party bytes, and writes
 *           nothing but its own stack snapshot and D_80146254. The 0x28-byte
 *           frame holds the eight-byte snapshot at $sp+0x10 plus the saved
 *           $ra at 0x20 and $s0/$s1 at 0x18/0x1C; the first loop keeps the
 *           snapshot base in $a0 because it calls nothing, the second keeps it
 *           in $s1 across the party-helper jal, each loop bounds its counter
 *           with `andi` plus `sltiu` against 3, the three arm immediates are
 *           materialised in $a0/$a1/$a2 before that jal with the zero fourth
 *           one in its delay slot, the teardown id 7 fills its own jal delay
 *           slot, and the final call keeps a plain nop slot. The address is
 *           entry 9 - the word at 0x801FBFDC, payload offset 0x53DC - of the
 *           thirteen-entry callback-pointer table at 0x801FBFB8 that
 *           dispatchRecordCallbackScenarioScena1300_801FB350 selects with the
 *           record's unsigned byte at offset 0x7A and calls with the record
 *           pointer and D_8014686C, so this handler runs for a record whose
 *           0x7A byte reads 9 and no in-image jal reaches the address.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the scaled stride sits in a separate `offset` local, the form
 * the exact sibling copyRecordPartyBytesThenInvokeHelperScenarioScena0900_801FE234
 * records for the same D_80145F09 stride: with the product inline in that
 * subscript gcc hoists the symbol address into a saved register.
 */
void invokePartyHelperForWorkRecordPartyBytesThenArmEffectGroupAndResetEffectScenarioScena1300_801FBDD8(
    void) {
  u8 slots[8];
  u8 i;
  u32 offset;

  for (i = 0; i < 3; i++) {
    offset = i * 0x140;
    slots[i] = D_80145F09[offset];
  }
  for (i = 0; i < 3; i++) {
    func_801C1ABC(slots[i]);
  }
  func_80166E88(7, 4, 2, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C1630();
}
