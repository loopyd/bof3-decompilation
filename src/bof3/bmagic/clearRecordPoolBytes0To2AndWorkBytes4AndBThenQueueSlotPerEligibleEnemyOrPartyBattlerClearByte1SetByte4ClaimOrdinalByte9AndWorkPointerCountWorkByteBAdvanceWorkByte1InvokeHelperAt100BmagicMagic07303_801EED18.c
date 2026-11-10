#include "bof3/bof3.h"

/* Base of the 0xA0-record pool this overlay keeps in its own image, and the two
 * bytes that follow it. The pool occupies 0x801F0670 through 0x801F21F0, the
 * last 0x1B80 bytes of the payload at stride 0x2C, and ends exactly at the
 * pointer cell 0x801F21F0 that the overlay publishes its stepped work record
 * through. The sibling lift reserveFirstFreeRecordBmagicMagic07303_801F03E8
 * walks the same base with the same 0x2C stride and tests record byte 0x00 as
 * the reservation flag, so this body's byte 0x00/0x01/0x02 clear of every
 * record is the pool reset that precedes the claims. Splat binds the three
 * addresses through this target's generated undefined_syms_auto.txt, so no
 * target-map row is needed. */
extern u8 D_801F0670[]; /* @source 0x801F0670 @kind data */
extern u8 D_801F0671[]; /* @source 0x801F0671 @kind data */
extern u8 D_801F0672[]; /* @source 0x801F0672 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, reached through this
 * address-derived symbol that the sibling lifts of this target declare. The
 * body reads the cell once per site - the two cleared work bytes, the handler
 * -index increment, the work pointer published into each claimed slot and the
 * count increment after it - because the stores through the loaded pointer may
 * alias the cell, so the original re-materializes the address with its own
 * `lui` / `lw` pair at each read. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle target byte at 0x80146384 in main executable RAM, outside this
 * overlay's image and bound by this target's generated undefined_syms_auto.txt.
 * The sibling lifts
 * raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C
 * and
 * clearRecordPoolBytes0To2AndWorkCountThenQueueSlotPerEligibleBattlerCopyTripletSetByte3StepByte9AndWorkPointerThenCopy15HalfwordsSettingBit15SetGlobalByte80145988AndAdvanceWorkByte1BmagicMagic09203_801EED04
 * read the same byte as the shared battle target, and in the latter bit 0x40
 * selects which roster is queued: set for the eight enemy slots, clear for the
 * three party slots, the same split this body performs. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Action-slot pool at 0x801EC330 in main RAM: 0x78-byte stride. The original
 * reaches each field through its own flat symbol - `lui $at,%hi(SYM)`,
 * `addu $at,$at,slotOffset`, store `%lo(SYM)($at)` - with the scaled index in
 * one register shared by all four accesses, so every addressed field keeps its
 * own address-anchored name. Sibling lifts of the family declare bytes
 * 0x801EC331, 0x801EC334 and 0x801EC339 and word 0x801EC3A4 of the same store:
 * this body clears slot byte 0x01, stamps the battler selector into slot byte
 * 0x04, writes the claim ordinal into slot byte 0x09 and publishes the
 * work-object pointer into slot word 0x74. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC334[]; /* @source 0x801EC334 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Battler-status helper of the battle image at 0x801DB524, outside this overlay
 * and bound through this target's generated undefined_funcs_auto.txt. It reports
 * whether one battler slot is unusable, so a zero answer means the slot is
 * eligible; the same test gates the claims of the sibling lifts of this family.
 * Its nonzero answer is used only as the skip guard. */
extern u32 func_801DB524(u32 arg0); /* @source 0x801DB524 */

/* Action-slot allocator of the shared main executable at 0x801E590C, outside
 * this overlay image. This body passes selector 1 and the request class 0x26
 * and uses the zero-extended low byte of the answer as the index of the slot it
 * claimed, the same two-step use the sibling lifts record for this allocator. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Main-executable helper at 0x8015D71C, outside this overlay image and bound
 * through this target's generated undefined_funcs_auto.txt. The bmagic siblings
 * call it with one code and use no part of an answer, so only its effect is
 * established and its role stays address-anchored. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EED18
 * @behavior Init stage 0 of this overlay. The frame entry func_801EEC28 copies
 *           the two-word pointer run {func_801EED18, func_801EEF50} into its own
 *           frame and calls the word selected by work-object byte 0x01, so this
 *           body runs while that byte is 0 and no jal in the payload names it.
 *           It first resets the overlay's record pool at 0x801F0670 - 0xA0
 *           records of stride 0x2C - by clearing record bytes 0x00, 0x01 and
 *           0x02 of every record with one sb per byte, computing the record
 *           offset as the counter times 0x2C. It then clears work bytes 0x04 and
 *           0x0B and increments work byte 0x01, which hands the next frame to
 *           the second entry of the run at 0x801EEF50. The body reads the shared
 *           battle target byte at 0x80146384 and, with bit 0x40 set, walks the
 *           eight enemy selectors 3 through 10, otherwise the three party
 *           selectors 0 through 2. A selector is skipped when the shared status
 *           helper func_801DB524 reports its slot unusable, called with the
 *           ordinal plus three masked to a byte on the enemy walk and with the
 *           ordinal itself masked to a byte on the party walk. Every other one
 *           claims an action slot through the shared allocator
 *           func_801E590C(1, 0x26), whose zero-extended low byte scaled by the
 *           0x78-byte stride reaches slot byte 0x01, slot byte 0x04, slot byte
 *           0x09 and slot word 0x74: byte 0x01 is cleared, byte 0x04 receives
 *           the selector, byte 0x09 receives the running count of claimed slots
 *           scaled by 0x1C and raised by one, and word 0x74 receives the
 *           work-object pointer read from the scratchpad cell 0x1F800044, after
 *           which work byte 0x0B is incremented through its own read of that
 *           cell. The slot stride is the `slot << 4`, then `- slot`, then
 *           `<< 3` chain of the original, which keeps the accumulator of the
 *           scale in $a0 - the register that also holds the slot offset - for
 *           all three instructions of both roster walks; the
 *           `(u8)allocator(...) * 0x78u` spelling gives that accumulator its
 *           own $v1 and differs in two instructions per walk. The 0x20-byte
 *           frame keeps $ra at 0x1C and $s0 to $s2 at 0x10 to 0x18 across the
 *           calls, and the body ends by invoking the main-executable helper
 *           func_8015D71C with 0x100. Takes no arguments and returns nothing.
 *           The roles of the allocator's request class 0x26, of the action-slot
 *           bytes and of helper code 0x100 remain unestablished, so they stay
 *           literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordPoolBytes0To2AndWorkBytes4AndBThenQueueSlotPerEligibleEnemyOrPartyBattlerClearByte1SetByte4ClaimOrdinalByte9AndWorkPointerCountWorkByteBAdvanceWorkByte1InvokeHelperAt100BmagicMagic07303_801EED18(
    void) {
  u8 index;
  u8 claimed;
  u8 *published;
  u8 *record;
  u32 offset;
  u32 slotOffset;
  u8 slot;

  claimed = 0;

  for (index = 0; index < 0xA0; index++) {
    offset = index * 0x2C;
    D_801F0670[offset] = 0;
    D_801F0671[offset] = 0;
    D_801F0672[offset] = 0;
  }

  D_1F800044[4] = 0;
  D_1F800044[0xB] = 0;
  D_1F800044[1]++;

  if (D_80146384 & 0x40) {
    for (index = 0; index < 8; index++) {
      if ((u8)func_801DB524((u8)(index + 3)) == 0) {
        slot = (u8)func_801E590C(1u, 0x26u);
        slotOffset = slot << 4;
        slotOffset -= slot;
        slotOffset <<= 3;
        published = D_1F800044;
        D_801EC331[slotOffset] = 0;
        D_801EC334[slotOffset] = (u8)(index + 3);
        D_801EC339[slotOffset] = claimed * 0x1C + 1;
        record = D_1F800044;
        *(u32 *)&D_801EC3A4[slotOffset] = (u32)published;
        record[0xB]++;
        claimed++;
      }
    }
  } else {
    for (index = 0; index < 3; index++) {
      if ((u8)func_801DB524(index) == 0) {
        slot = (u8)func_801E590C(1u, 0x26u);
        slotOffset = slot << 4;
        slotOffset -= slot;
        slotOffset <<= 3;
        published = D_1F800044;
        D_801EC331[slotOffset] = 0;
        D_801EC334[slotOffset] = index;
        D_801EC339[slotOffset] = claimed * 0x1C + 1;
        record = D_1F800044;
        *(u32 *)&D_801EC3A4[slotOffset] = (u32)published;
        record[0xB]++;
        claimed++;
      }
    }
  }

  func_8015D71C(0x100);
}
