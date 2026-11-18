#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell shared by the battle, bmagic, world and
 * boss overlays; this target's map binds the row. The original re-materialises
 * the cell with its own lui 0x1F80 / lw 0x44 pair before every store group in
 * this body, because a store through the loaded pointer may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay image; this
 * target's generated undefined_syms_auto.txt already binds the address. This
 * body only reads its halfword 0x36, its word 0x38 and its halfword 0x3E: the
 * vertical position word 0x38 and the distance measure 0x3E are the offsets the
 * exact same-target sibling clearWorkByte24Byte2AAndMode48SetByte29To5Apply
 * Lookup15CRequestResource6CopyBattleRecordPositionSeedDistanceThenAdvanceStep
 * BossBoss02816_800C2024 reads too, with halfword 0x36 here in place of that
 * sibling's word 0x34. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-executable lookup and resource helpers, bound by this target's
 * generated undefined_funcs_auto.txt rows. The exact sibling named above
 * declares the same one-argument prototypes for both: 0x8014DD3C applies a
 * main lookup record and 0x8014D6B8 requests a resource id. */
extern void func_8014DD3C(s32 entry);    /* @source 0x8014DD3C */
extern void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* Action-slot pool of the battle image at 0x801EC330 in main RAM, outside this
 * overlay: 0x78-byte stride, with each field reached through its own flat
 * symbol - `lui $at,%hi(SYM)`, `addu $at,$at,index*0x78`, store `%lo(SYM)($at)`
 * - and the scaled index in one register shared by both stores. The pool's
 * work-object pointer word sits at slot offset +0x74, which is the address the
 * exact sibling queueActionSlotSetByte4Byte9AndByteAAndPublishWorkPointerThen
 * CountWorkByteBAndIncrementWorkByte1BmagicMagic08803_801EF1AC publishes the
 * work record through. The slot byte at +0x01 keeps its address anchor: no
 * other lifted source in the tree writes that byte, and the sibling
 * queueTwoLinkedSlotsCopyRecordTripletsThenInvokeHelperAt100AndAdvanceWork
 * Byte1BmagicMagic07803_801EECFC clears it to 0. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Slot allocator of the battle image at 0x801E590C, outside this overlay and
 * bound through this target's generated undefined_funcs_auto.txt. It returns
 * the index of the claimed action slot, which this body truncates to a byte
 * before scaling it by the 0x78-byte stride. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x800C1C9C
 * @behavior Entry 0 of this overlay's seven-slot work-state pointer run based
 *           at 0x800C21AC, whose words are 0x800C1C9C, 0x800C1D90, 0x800C1E80,
 *           0x800C1F2C, 0x800C2024, 0x800C20C8 and 0x800C2120: the dispatcher
 *           dispatchWorkByte2Table21acThenInvokeSharedServiceWhenWorkByte0Bit0
 *           SetBossBoss02816_800C1C30 indexes that base with the work record
 *           byte 2 scaled by four, and no jal in the payload names this span,
 *           so the prologue and the return path place its boundary.
 *           It clears the work record published through the scratchpad pointer
 *           cell 0x1F800044 at bytes 0x24, 0x2A and 0x48 and sets that record's
 *           byte 0x29 to 2, then applies main lookup record 0x15C through
 *           func_8014DD3C and requests resource 4 through func_8014D6B8. Each
 *           clear and the byte 0x29 store re-reads the cell, since a store
 *           through the loaded pointer may alias it; the 2 is loaded into $v0
 *           before the first call and that store fills the call's delay slot,
 *           while the 4 is materialised in the second call's delay slot.
 *           It then copies the shared battle record's halfword 0x36 plus seven
 *           and its word 0x38 into the work record at the same offsets, stores
 *           -0x20 in work word 0x20 and zero in work word 0x14 - the per-frame
 *           delta and the accumulator the run's entry 1 at 0x800C1D90 adds -
 *           sets work byte 9 to 6, the frame counter that same entry
 *           decrements, and seeds work halfword 0x3E from the record's own 0x3E
 *           plus 0x1000, above the record's measured distance.
 *           It then claims an action slot of the 0x78-byte pool: it truncates
 *           the allocator's returned index func_801E590C(3, 2) to a byte,
 *           scales it by the stride into one register shared by both stores,
 *           stamps the slot byte at 0x801EC331 with 1 and publishes the work
 *           record pointer in the slot's +0x74 word at 0x801EC3A4, re-reading
 *           the scratchpad cell for that pointer and for the tail. Finally it
 *           advances work byte 2 by one at byte width, handing the next frame
 *           of the run to entry 1 at 0x800C1D90.
 *           The roles of work bytes 0x24, 0x2A, 0x29 and 0x48 and of work words
 *           0x20 and 0x14 stay offset-anchored: this lift establishes the
 *           stores and their widths, not what those fields select. The 0x18-byte
 *           frame keeps $ra at 0x10 across the three calls, the body takes no
 *           arguments, and the value left in $v0 is the incremented step byte,
 *           which the indirect step call discards.
 *           Representation notes: the first seed wrote the word 0x38 copy and
 *           the distance seed inline and measured 50/61 instructions, its first
 *           difference at instruction 28, where it held the copied word in $a0
 *           and stored it after the byte 9 store instead of the original's
 *           $v0 pair placed before it; reading the record halfword 0x3E into
 *           the local distance first, and reloading the cell for the tail into
 *           its own local rather than the work pointer, moved those copies back
 *           to $v0 and the tail pointer to $a0 and measured 61/61.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte24Byte2AAndMode48SetByte29To2Lookup15CResource4CopyRecordPositionSeedDistancePlus1000SetWord20Minus20Word14ZeroCountdown6ClaimSlotPublishWorkPointerThenAdvanceStepBossBoss02816_800C1C9C(
    void) {
  u8 *record;
  u8 *work;
  u8 *published;
  u16 distance;
  u32 slot;

  D_1F800044[0x24] = 0;
  D_1F800044[0x2A] = 0;
  D_1F800044[0x48] = 0;
  D_1F800044[0x29] = 2;
  func_8014DD3C(0x15C);
  func_8014D6B8(4);
  record = D_801EB4E0;
  work = D_1F800044;
  *(u16 *)(work + 0x36) = *(u16 *)(record + 0x36) + 7;
  *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38);
  distance = *(u16 *)(record + 0x3E);
  *(s32 *)(work + 0x20) = -0x20;
  *(s32 *)(work + 0x14) = 0;
  work[9] = 6;
  *(u16 *)(work + 0x3E) = distance + 0x1000;
  slot = (u8)func_801E590C(3, 2) * 0x78;
  D_801EC331[slot] = 1;
  published = D_1F800044;
  *(u8 **)&D_801EC3A4[slot] = published;
  published[2]++;
}
