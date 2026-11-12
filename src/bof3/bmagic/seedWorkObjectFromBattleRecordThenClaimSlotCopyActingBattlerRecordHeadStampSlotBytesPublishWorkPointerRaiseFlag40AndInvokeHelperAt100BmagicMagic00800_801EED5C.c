#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at the low scratchpad address this
 * target's map already names. The original re-materialises the cell address
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) before every read of the cell, because a
 * store through the pointer it supplied may alias the cell itself, so the cell
 * is declared volatile as in the exact same-target sibling 0x801EF7C8. */
extern u8 *volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The body reads the record in three groups: byte 8 and the word triplet
 * 0x34/0x38/0x3C at the head, and byte 0 at the tail, and re-reads the cell
 * after the stores that may alias it. Splat binds the address through this
 * target's generated undefined_syms_auto.txt, so no target-map row is added. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler selector of the main executable. Party records are indexed 0
 * to 2 and enemy records by selector minus 3, so the unsigned compare against 3
 * below splits the two record arrays. Splat binds the address through this
 * target's generated undefined_syms_auto.txt. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Party record array at 0x80145E90, in main executable RAM outside this
 * overlay's image, scaled by the 0x140-byte party stride. The 0x74-byte head
 * this body copies out of it is the record image the same-target sibling
 * 0x801EF7C8 copies into the same slot store. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Enemy record array at 0x801EB630, in main executable RAM outside this
 * overlay's image, scaled by the 0x118-byte enemy stride; the acting battler
 * selector minus three indexes it. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */

/* Queued slot store at 0x801EC330 in main executable RAM, whose 0x78-byte
 * stride this body scales the allocated index by to reach the copy destination.
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

/* Shared slot-4 event dispatcher of the battle overlays at 0x800AEF98, outside
 * this overlay's image and owned by the battle targets. This lift only calls
 * it, so the prototype follows the call site: the first argument is a
 * byte-sized event selector and the second a byte-sized value, and no result is
 * used. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is added here. */
extern void func_800AEF98(s32 event, s32 value); /* @source 0x800AEF98 @kind function */

/* Slot allocator at 0x801E590C, outside this overlay's image: it claims the
 * first entry of the queued slot store whose byte +0x00 has bit 0 clear, sets
 * that bit, stores the two identity bytes at +0x06 and +0x05 and returns the
 * entry index, or 0xFF when every entry is already active. This body uses the
 * low byte of the returned word without testing that failure code. */
extern u32 func_801E590C(u32 mode0, u32 mode1); /* @source 0x801E590C @kind function */

/* Selector-taking helper of the main executable at 0x8015D71C, outside this
 * overlay's image. The bmagic lifts pass it a constant selector (0x100 in
 * seedMotionTintAndChannelFieldsThenAdvanceWorkByte2BmagicMagic00800_801F05FC
 * and here, 0x101 in
 * invokeHelperAt101SetByte9ToFAndAdvanceWorkByte2WhenHelperReadyBmagicMagic00800
 * _801F047C) and declare the same one-argument prototype. Splat binds the
 * address through this target's generated undefined_funcs_auto.txt, so no
 * target-map row is added here. */
void func_8015D71C(s32 arg0);

/* The 0x74-byte record head this body moves from a battler record into the
 * claimed slot entry; the original moves it as one block, 28 words in a
 * four-word loop plus the trailing word, which is the expansion of one 29-word
 * structure assignment. */
typedef struct BmagicMagic00800QueuedRecordHead {
  u32 words[29];
} BmagicMagic00800QueuedRecordHead;

/* @source 0x801EED5C
 * @behavior Work byte 1 state 0 of this overlay: entry 0 of the four-entry code
 *           pointer table at 0x801EEC04 (0x801EED5C, 0x801EEF6C, 0x801EEFDC,
 *           0x801EF010), which 0x801EECF0 dispatches on the scratchpad work
 *           object's state byte 1. It first copies the shared battle work
 *           record's byte 8 and word triplet 0x34/0x38/0x3C from the pointer
 *           cell at 0x801EB4E0 into the work object, clears work bytes 0xB and
 *           9, increments work byte 1 with a byte-width add, and invokes the
 *           shared battle helper at 0x800AEF98 with the event selector 0xC and
 *           the value 2. It then claims one slot through the allocator at
 *           0x801E590C with the identity bytes 1 and 0x1F and copies the acting
 *           battler's 0x74-byte record head into the claimed entry of the
 *           0x78-byte-stride slot store at 0x801EC330. Selectors below 3 copy
 *           from the 0x140-byte-stride party records at 0x80145E90, and other
 *           selectors from the 0x118-byte-stride enemy records at 0x801EB630
 *           indexed by selector minus 3; the selector is the acting battler
 *           byte at 0x80146374. It stamps the entry byte +0x06 with 1, +0x01
 *           and +0x02 with 0 and +0x05 with 0x1F, publishes the scratchpad
 *           work-object pointer in the entry word +0x74, raises bit 0x40 of the
 *           shared work record's byte 0, and finally invokes the selector-taking
 *           helper at 0x8015D71C with the selector 0x100. The allocator's 0xFF
 *           failure code is not tested. The 0x18-byte frame only keeps $ra
 *           across the three calls; nothing is returned and no other state is
 *           read or written. The roles of the stamped entry bytes stay
 *           structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkObjectFromBattleRecordThenClaimSlotCopyActingBattlerRecordHeadStampSlotBytesPublishWorkPointerRaiseFlag40AndInvokeHelperAt100BmagicMagic00800_801EED5C(
    void) {
  u8 *work;
  u8 *work2;
  u8 *record;
  u32 slot;
  u32 offset;
  u32 word3C;
  u8 selector;

  g_battle_work[8] = D_801EB4E0[8];
  work = g_battle_work;
  *(u32 *)&work[0x34] = *(u32 *)&D_801EB4E0[0x34];
  *(u32 *)&work[0x38] = *(u32 *)&D_801EB4E0[0x38];
  word3C = *(u32 *)&D_801EB4E0[0x3C];
  work[0xB] = 0;
  work2 = g_battle_work;
  *(u32 *)&work[0x3C] = word3C;
  work2[9] = 0;
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
  offset = (u8)slot * 0x78;
  work = g_battle_work;
  D_801EC336[offset] = 1;
  D_801EC331[offset] = 0;
  D_801EC332[offset] = 0;
  D_801EC335[offset] = 0x1F;
  record = D_801EB4E0;
  *(u32 *)&D_801EC3A4[offset] = (u32)work;
  record[0] |= 0x40;
  func_8015D71C(0x100);
}
