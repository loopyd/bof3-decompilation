#include "bof3/bof3.h"

/* Record flags pool of this overlay at 0x801F0454: 0x60 records of stride 0x20,
 * so the pool ends exactly at 0x801F1054, the work-record pointer cell the frame
 * loop func_801EEC18 publishes into. The reset below clears record bytes 0x00,
 * 0x01 and 0x02 of every record, so the original addresses the pool base and the
 * two bytes that follow it with their own `lui` / `addu` / `sb` triple; the row
 * in this target's symbols.txt names the base and the generated
 * undefined_syms_auto.txt binds the other two. */
extern u8 magic06303RecordFlags[]; /* @source 0x801F0454 @kind data */
extern u8 D_801F0455[]; /* @source 0x801F0455 @kind data */
extern u8 D_801F0456[]; /* @source 0x801F0456 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work like the siblings of this target declare it. The body reads
 * the cell at every site - the two cleared bytes, the handler-index increment,
 * the work pointer published into each claimed slot and the count increment
 * after it - because the stores through the loaded pointer may alias the cell,
 * so the original re-materializes the address with its own `lui` / `lw` pair at
 * each read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle selection byte at 0x80146374 in main executable RAM, outside
 * this overlay image and bound by this target's generated
 * undefined_syms_auto.txt. Byte 0 holds the acting battler: the sibling
 * queueSlotPerEligibleEnemyAndPartyBattlerSetByte1To2OrdinalByte4SelectorByteBA
 * ndRecordPointerBmagicMagic06903_801F0274 rejects a scanned selector when it
 * equals this byte, and this body does the same for both of its scans. */
extern u8 D_80146374[]; /* @source 0x80146374 @kind data */

/* Action-slot store at 0x801EC330 in main RAM: 0x78-byte stride. The original
 * reaches each field through its own flat symbol - `lui $at,%hi(SYM)`,
 * `addu $at,$at,index*0x78`, store `%lo(SYM)($at)` - with the scaled index in
 * one register shared by all three stores, so every addressed field keeps its
 * own address-anchored name, exactly as the sibling lift
 * claimRecordClearByte1StoreWorkPointerPublishSavedPointerSetWorkByteBToOneAndAd
 * vanceHandlerIndexBmagicMagic08103_801EEEDC declares bytes 0x801EC331 and word
 * 0x801EC3A4 of the same store. This body clears slot byte 0x01, stamps the
 * battler selector into slot byte 0x04 and publishes the work-object pointer into
 * slot word 0x74. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC334[]; /* @source 0x801EC334 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Battler-status helper of the battle image at 0x801DB524, outside this overlay
 * and bound through this target's generated undefined_funcs_auto.txt. It reports
 * whether one battler slot is unavailable - inactive or carrying the 0x4000 flag
 * - so a zero answer means the slot is eligible, the reading the sibling
 * seedWorkObjectFromBattlerRecordWhenBattlerReadyAndStateByteNotSixElseFinishWor
 * kByte2BmagicMagic01200_801EF4B8 records. */
extern u8 func_801DB524(u8 arg0); /* @source 0x801DB524 */

/* Action-slot allocator of the shared main executable at 0x801E590C, outside
 * this overlay image. This body passes selector 1 and sub-selector 0x68 and uses
 * the zero-extended low byte of the answer as the index of the slot it claimed,
 * the same two-step use the sibling lifts of this target record for this
 * allocator. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Main-executable helper at 0x8015D71C, outside this overlay image and bound
 * through this target's generated undefined_funcs_auto.txt. The bmagic siblings
 * call it with one code and use no part of an answer, so only its effect is
 * established and its role stays address-anchored. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EECF8
 * @behavior Init stage 0 of this overlay. The frame entry func_801EEC18 copies
 *           the two-word pointer run {func_801EECF8, func_801EEEE4} into its
 *           frame and calls the word selected by work-object byte 0x01, so this
 *           body runs while that byte is 0. It first resets the flags pool at
 *           0x801F0454 - 0x60 records of stride 0x20 - by clearing record bytes
 *           0x00, 0x01 and 0x02 of every record, then clears work bytes 0x0B and
 *           0x09 and increments work byte 0x01, which hands the next frame to
 *           the second stage at 0x801EEEE4. Work byte 0x0B is the queue count:
 *           the second stage tests it and, when it is still zero, raises bit 0x4
 *           of the battle halfword at 0x801462E8, and this body increments it
 *           once per claimed slot. The body then scans the eight enemy battler
 *           selectors 3 through 10 and then the three party selectors 0 through
 *           2. A selector is skipped when the shared status helper func_801DB524
 *           reports its slot unavailable or when it equals the acting battler
 *           byte at 0x80146374; every other one claims an action slot through the
 *           shared allocator func_801E590C(1, 0x68), whose zero-extended low byte
 *           scaled by the 0x78-byte stride reaches slot byte 0x01, slot byte 0x04
 *           and slot word 0x74: byte 0x01 is cleared, byte 0x04 receives the
 *           selector and word 0x74 receives the work-object pointer read from the
 *           scratchpad cell 0x1F800044, after which work byte 0x0B is
 *           incremented. Both scans run their whole span whether or not a
 *           selector is accepted, and the body ends by invoking the
 *           main-executable helper func_8015D71C with 0x100. The 0x20-byte frame
 *           keeps $ra at 0x18, $s1 at 0x14 and $s0 at 0x10 across the calls; the
 *           roles of the allocator's selector 0x68, of slot bytes 0x01 and 0x04
 *           and of helper code 0x100 remain unestablished, so those constants
 *           stay literal.
 * @status partial
 * @match 95.12
 * @residual 117/123 instructions at the original 492-byte image, so the
 *           candidate keeps the original length and differs in six register
 *           fields only. The first difference is +0x00D0: the work-object
 *           pointer published into slot word 0x74 is loaded into $a1 where the
 *           original keeps it in $v0, so that publication's `lui`, `lw` and
 *           `sw` name $a1 - three instructions in each of the two roster scans.
 *           The frame, the {ra,s0,s1} save set, the pool reset, both induction
 *           variables, both guards, the allocator's 0x78 scale chain and every
 *           other store match. gcc 2.7.2's global-alloc dump (-dg) settles that
 *           value's allocno (73) on $a1 while listing $v0 and $v1 as conflicts,
 *           and the exact sibling
 *           decrementWorkByte9ThenQueueSlotSetByte1To3PublishWorkPointerAndAdvan
 *           ceWorkByte2WhenZeroBmagicMagic15103_801EFBAC reaches the same $a1
 *           for the same publication with a cell read of its own; the source
 *           spelling that frees $v0 for it is not established.
 *           One local per cell read (published, record) is required: a single
 *           local or an inline cell read at the store site lets the reload pass
 *           fold the two reads into one and drops the body to 476 bytes.
 */
void clearRecordPoolBytes0To2AndWorkBytesBAnd9ThenQueueSlotPerEligibleEnemyAndPartyBattlerClearByte1SetBattlerByte4AndWorkPointerCountWorkByteBAdvanceWorkByte1InvokeHelperAt100BmagicMagic06303_801EECF8(
    void) {
  u8 index;
  u8 *published;
  u8 *record;
  u32 offset;
  u32 slotOffset;

  for (index = 0; index < 0x60; index++) {
    offset = index * 0x20;
    magic06303RecordFlags[offset] = 0;
    D_801F0455[offset] = 0;
    D_801F0456[offset] = 0;
  }

  g_battle_work[0xB] = 0;
  g_battle_work[9] = 0;
  g_battle_work[1]++;

  for (index = 0; index < 8; index++) {
    if (((u8)func_801DB524((u8)(index + 3)) == 0u) &&
        (D_80146374[0] != (index + 3))) {
      slotOffset = ((u8)func_801E590C(1u, 0x68u)) * 0x78u;
      published = g_battle_work;
      D_801EC331[slotOffset] = 0;
      D_801EC334[slotOffset] = (u8)(index + 3);
      record = g_battle_work;
      *(u32 *)&D_801EC3A4[slotOffset] = (u32)published;
      record[0xB]++;
    }
  }

  for (index = 0; index < 3; index++) {
    if (((u8)func_801DB524(index) == 0u) && (D_80146374[0] != index)) {
      slotOffset = ((u8)func_801E590C(1u, 0x68u)) * 0x78u;
      published = g_battle_work;
      D_801EC331[slotOffset] = 0;
      D_801EC334[slotOffset] = index;
      record = g_battle_work;
      *(u32 *)&D_801EC3A4[slotOffset] = (u32)published;
      record[0xB]++;
    }
  }

  func_8015D71C(0x100);
}
