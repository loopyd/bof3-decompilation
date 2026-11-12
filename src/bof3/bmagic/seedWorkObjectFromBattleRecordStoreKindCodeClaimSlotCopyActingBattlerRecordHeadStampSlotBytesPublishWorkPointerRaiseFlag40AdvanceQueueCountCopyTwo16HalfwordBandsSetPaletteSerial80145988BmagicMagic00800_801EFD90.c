#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at the low scratchpad address this
 * target's map already names. The original re-materialises the cell address
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) before every read of the cell, because a
 * store through the pointer it supplied may alias the cell itself, so the cell
 * is declared volatile like the exact sibling
 * seedWorkObjectFromBattleRecordThenClaimSlotCopyActingBattlerRecordHeadStamp
 * SlotBytesPublishWorkPointerAndRaiseFlag40BmagicMagic00800_801EF7C8. */
extern u8 *volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The body reads byte 8 and the word triplet 0x34/0x38/0x3C at the head and
 * byte 0 at the tail. Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler selector of the main executable. Party records are indexed 0
 * to 2 and enemy records by selector minus 3, so the unsigned compare against 3
 * below splits the two record arrays. Splat binds the address through this
 * target's generated undefined_syms_auto.txt. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Party record array at 0x80145E90, in main executable RAM outside this
 * overlay's image, scaled by the 0x140-byte party stride. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Enemy record array at 0x801EB630, in main executable RAM outside this
 * overlay's image, scaled by the 0x118-byte enemy stride; the acting battler
 * selector minus three indexes it. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */

/* Queued slot store at 0x801EC330 in main executable RAM, whose 0x78-byte
 * stride this body scales the claimed index by to reach the copy destination.
 * The original reaches each stamped field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,offset` + `sb|sw %lo(ADDR)($at)`, so every addressed field keeps its
 * own flat symbol rather than one shared record base: bytes +0x01, +0x02, +0x05
 * and +0x06 and the pointer word at +0x74. */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC335[]; /* @source 0x801EC335 @kind data */
extern u8 D_801EC336[]; /* @source 0x801EC336 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-RAM serial byte at 0x80145988. The AREA030 color-row builder
 * publishes the same 1 after it rewrites a color row, so this body's own
 * halfword-table copy ends with it; the byte's role beyond that stays
 * structural. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Shared slot-4 event dispatcher of the battle overlays at 0x800AEF98, outside
 * this overlay's image and owned by the battle targets. This lift only calls
 * it, so the prototype follows the call site and no result is used. Splat binds
 * the address through this target's generated undefined_funcs_auto.txt. */
extern void func_800AEF98(s32 event, s32 value); /* @source 0x800AEF98 @kind function */

/* Slot allocator at 0x801E590C, outside this overlay's image: it claims the
 * first entry of the queued slot store whose byte +0x00 has bit 0 clear, sets
 * that bit, stores the two identity bytes at +0x06 and +0x05 and returns the
 * entry index, or 0xFF when every entry is already active. This body uses the
 * low byte of the returned word without testing that failure code. */
extern u32 func_801E590C(u32 mode0, u32 mode1); /* @source 0x801E590C @kind function */

/* Battle kind result code resolver of this overlay at 0x801F0230; it returns 7
 * when the shared battle-kind halfword equals 7 and 0xFF otherwise. */
extern u8 resolveKindResultCodeBmagicMagic00800_801F0230(void); /* @source 0x801F0230 */

/* The 0x74-byte record head this body moves from a battler record into the
 * claimed slot entry; the original moves it as one block, 28 words in a
 * four-word loop plus the trailing word, which is the expansion of one 29-word
 * structure assignment. */
/* The two 0x10-halfword bands this body copies from the shared table at
 * 0x80036C00 into the table at 0x8003AC00. The original reaches the second band
 * as a +0x20 displacement from the first band's base register, the expansion of
 * one access per member of this two-member type rather than of two separately
 * materialised table bases. The roles of both tables stay structural. */
typedef struct BmagicMagic00800HalfwordBand {
  u16 lead[0x10];
  u16 tail[0x10];
} BmagicMagic00800HalfwordBand;

typedef struct BmagicMagic00800QueuedRecordHead {
  u32 words[29];
} BmagicMagic00800QueuedRecordHead;

/* @source 0x801EFD90
 * @behavior Work byte 1 state 0 of the overlay handler run at 0x801EFD30:
 *           entry 0 of the three-entry code pointer table at 0x801EEC4C
 *           (0x801EFD90, 0x801F0018, 0x801F01E4), which that dispatcher copies
 *           into its own frame and selects with the scratchpad work object's
 *           state byte 1. It copies the shared battle work record's byte 8 and
 *           words 0x34/0x38/0x3C from the pointer cell at 0x801EB4E0 into the
 *           work object, stores the kind result code returned by 0x801F0230 in
 *           work byte 0x4, clears work byte 0xB, and increments work byte 1
 *           with a byte-width add, so the next frame runs 0x801F0018. It then
 *           invokes the shared battle helper at 0x800AEF98 with the event
 *           selector 0xC and the value 2 and claims one slot through the
 *           allocator at 0x801E590C with the identity bytes 1 and 0x1F. The
 *           acting battler byte at 0x80146374 selects the source of the
 *           0x74-byte record head copied into the claimed entry of the
 *           0x78-byte-stride slot store at 0x801EC330: selectors below 3 copy
 *           from the 0x140-byte-stride party records at 0x80145E90, and other
 *           selectors from the 0x118-byte-stride enemy records at 0x801EB630
 *           indexed by selector minus 3. It stamps the entry byte +0x01 with 2,
 *           +0x06 with 1, +0x02 with 0 and +0x05 with 0x1F, publishes the
 *           scratchpad work-object pointer in the entry word +0x74, increments
 *           work byte 0xB, the queue count the sibling lift at 0x801EED48
 *           names, and raises bit 0x40 of the shared work record's byte 0. Last
 *           it copies the 0x20 halfwords of the shared table at 0x80036C00 into
 *           the table at 0x8003AC00, first the leading sixteen entries and then
 *           the sixteen at +0x20, and publishes 1 at the shared main-RAM byte
 *           0x80145988. The allocator's 0xFF failure code is not tested. The
 *           0x18-byte frame only keeps $ra across the three calls; nothing is
 *           returned and no other state is read or written. The roles of the
 *           stamped entry bytes, of the two halfword tables and of work byte 4
 *           beyond the result code stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkObjectFromBattleRecordStoreKindCodeClaimSlotCopyActingBattlerRecordHeadStampSlotBytesPublishWorkPointerRaiseFlag40AdvanceQueueCountCopyTwo16HalfwordBandsSetPaletteSerial80145988BmagicMagic00800_801EFD90(
    void) {
  u8 index;
  u8 *work;
  u32 slot;
  u32 offset;
  u8 selector;
  u8 *counter;
  BmagicMagic00800HalfwordBand *bandSrc;
  BmagicMagic00800HalfwordBand *bandDst;

  g_battle_work[8] = D_801EB4E0[8];
  work = g_battle_work;
  *(u32 *)&work[0x34] = *(u32 *)&D_801EB4E0[0x34];
  *(u32 *)&work[0x38] = *(u32 *)&D_801EB4E0[0x38];
  *(u32 *)&work[0x3C] = *(u32 *)&D_801EB4E0[0x3C];
  g_battle_work[4] = resolveKindResultCodeBmagicMagic00800_801F0230();
  g_battle_work[0xB] = 0;
  g_battle_work[1] += 1;
  func_800AEF98(0xC, 2);
  slot = func_801E590C(1, 0x1F);
  selector = D_80146374;
  if (selector < 3) {
    BmagicMagic00800QueuedRecordHead *src =
        (BmagicMagic00800QueuedRecordHead *)&D_80145E90[selector * 0x140];
    BmagicMagic00800QueuedRecordHead *dst =
        (BmagicMagic00800QueuedRecordHead *)&D_801EC330[(u8)slot * 0x78];
    *dst = *src;
  } else {
    BmagicMagic00800QueuedRecordHead *src =
        (BmagicMagic00800QueuedRecordHead *)&D_801EB630[(selector - 3) * 0x118];
    BmagicMagic00800QueuedRecordHead *dst =
        (BmagicMagic00800QueuedRecordHead *)&D_801EC330[(u8)slot * 0x78];
    *dst = *src;
  }
  index = 0;
  offset = (u8)slot * 0x78;
  work = g_battle_work;
  D_801EC331[offset] = 2;
  D_801EC336[offset] = 1;
  D_801EC332[offset] = 0;
  D_801EC335[offset] = 0x1F;
  counter = g_battle_work;
  *(u32 *)&D_801EC3A4[offset] = (u32)work;
  counter[0xB] += 1;
  D_801EB4E0[0] |= 0x40;
  bandDst = PSX_PTR(BmagicMagic00800HalfwordBand, 0x8003AC00u);
  bandSrc = PSX_PTR(BmagicMagic00800HalfwordBand, 0x80036C00u);
  for (; index < 0x10; index++) {
    bandDst->lead[index] = bandSrc->lead[index];
    bandDst->tail[index] = bandSrc->tail[index];
  }
  D_80145988 = 1;
}
