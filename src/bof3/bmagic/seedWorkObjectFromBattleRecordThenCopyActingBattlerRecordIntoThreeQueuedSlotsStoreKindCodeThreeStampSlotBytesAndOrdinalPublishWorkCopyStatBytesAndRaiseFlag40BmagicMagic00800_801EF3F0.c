#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at the low scratchpad address this
 * target's map already names. The original re-materialises the cell address
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) before every read of the cell, because a
 * store through the pointer it supplied may alias the cell itself, so the cell
 * is declared volatile like the sibling lift at 0x801EF7C8, which settled the
 * spelling against the same replay of the cell. */
extern u8 *volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The body reads the record in three groups: byte 8 and the word triplet
 * 0x34/0x38/0x3C at the head, bytes 0x28 and 0x24 in the tail, and byte 0 for
 * the flag store, and re-reads the cell after the stores that may alias it.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler selector of the main executable, re-read on every pass because
 * the slot allocator call may write it. Party records are indexed 0 to 2 and
 * enemy records by selector minus 3, so the unsigned compare against 3 below
 * splits the two record arrays. Splat binds the address through this target's
 * generated undefined_syms_auto.txt. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Party record array at 0x80145E90, in main executable RAM outside this
 * overlay's image, scaled by the 0x140-byte party stride. The 0x74-byte head
 * this body copies out of it is the same record image the exact sibling above
 * copies into the same slot store at 0x801EF7C8. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Enemy record array at 0x801EB630, in main executable RAM outside this
 * overlay's image, scaled by the 0x118-byte enemy stride; the acting battler
 * selector minus three indexes it. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */

/* Queued slot store at 0x801EC330 in main executable RAM, whose 0x78-byte
 * stride this body scales the allocated index by to reach the copy destination.
 * The original reaches each stamped field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,offset` + `sb|sw %lo(ADDR)($at)`, so every addressed field keeps its
 * own flat symbol rather than one shared record base: bytes +0x01, +0x02, +0x05,
 * +0x06 and +0x0B and the pointer word at +0x74. */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC335[]; /* @source 0x801EC335 @kind data */
extern u8 D_801EC336[]; /* @source 0x801EC336 @kind data */
extern u8 D_801EC33B[]; /* @source 0x801EC33B @kind data */
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
 * low byte of the returned word on every pass without testing that failure
 * code. */
extern u32 func_801E590C(u32 mode0, u32 mode1); /* @source 0x801E590C @kind function */

/* Main-executable helpers at 0x800AFB48 and 0x800AF8F0, outside this overlay's
 * image. Both read the shared battle work record at bytes 0x28, 0x27 and 0x24,
 * the three bytes this body publishes into the work object immediately before
 * and after them; the first returns a byte that this call site stores, the
 * second returns nothing. A search of src/ and include/ found no lift of either
 * when this seed was written, so both keep their address-qualified names. */
extern u32 func_800AFB48(u8 *record); /* @source 0x800AFB48 @kind function */
extern void func_800AF8F0(u8 *work); /* @source 0x800AF8F0 @kind function */

/* The 0x74-byte record head this body moves from a battler record into a
 * queued slot entry; the original moves it as one block, 28 words in a
 * four-word loop plus the trailing word, which is the expansion of one 29-word
 * structure assignment, the same block the sibling at 0x801EF7C8 copies. */
typedef struct BmagicMagic00800QueuedRecordHead {
  u32 words[29];
} BmagicMagic00800QueuedRecordHead;

/* @source 0x801EF3F0
 * @behavior Entry 0 of the four-entry code pointer table at 0x801EEC20
 *           (0x801EF3F0, 0x801EF6AC, 0x801EF728, 0x801EF010), which
 *           0x801EF384 dispatches on the scratchpad work object's state byte 1.
 *           It first copies the shared battle work record's byte 8 and its word
 *           triplet 0x34/0x38/0x3C from the pointer cell at 0x801EB4E0 into the
 *           work object, clears work bytes 0xB and 9, increments work byte 1 and
 *           invokes the shared battle helper at 0x800AEF98 with the event
 *           selector 0xC and the value 2. It then makes three passes over the
 *           queued slot store: each pass claims a slot through
 *           func_801E590C(1, 0x1F), selects the acting battler's record by the
 *           selector byte at 0x80146374 - party records at 0x80145E90 with the
 *           0x140-byte party stride for selectors 0 to 2, enemy records at
 *           0x801EB630 with the 0x118-byte enemy stride for selector 3 and above
 *           indexed by selector minus 3 - and copies that record's 0x74-byte
 *           head into the claimed entry of the 0x78-byte-stride slot store at
 *           0x801EC330. Each pass then stamps the entry byte +0x01 with 3, +0x06
 *           with 1, +0x02 with 0, +0x05 with 0x1F and +0x0B with the pass
 *           ordinal, publishes the work-object pointer in the entry word +0x74
 *           and increments the work object's byte 0xB. After the three passes it
 *           stores the byte func_800AFB48 returns for the shared record at work
 *           byte 0x27, copies record bytes 0x28 and 0x24 into the same work
 *           bytes, calls func_800AF8F0 on the work object and raises bit 0x40 of
 *           the shared work record's byte 0. The allocator's 0xFF failure code is
 *           not tested. The 0x20-byte frame keeps $ra at 0x18 and $s0/$s1 at
 *           0x10/0x14 across the calls, and the two pointer-cell loads that
 *           precede the frame are the entry reads the scheduler hoisted above the
 *           stack adjustment. Takes no arguments and returns nothing. The roles
 *           of the stamped entry bytes and of record bytes 8, 0x24, 0x27, 0x28,
 *           0x34, 0x38 and 0x3C are not established, so the name describes the
 *           measured effect and those fields stay offset-anchored.
 * @status partial
 * @match 97.14
 * @residual 170/175 instructions, 700 bytes. The only difference is one head
 *           group at +0x0054: the shared work record's word 0x3C is loaded into
 *           $v0 rather than $v1 and the pointer-cell read that follows into $a2
 *           rather than $v0, so the group's two stores keep $v0 and $a2 instead
 *           of $v1 and $v0. Both record-copy loops, the three slot stamps, the
 *           work-pointer publication, the pass ordinal and the whole tail match,
 *           so the residual is register allocation inside one store group and
 *           not statement shape or order.
 */
void seedWorkObjectFromBattleRecordThenCopyActingBattlerRecordIntoThreeQueuedSlotsStoreKindCodeThreeStampSlotBytesAndOrdinalPublishWorkCopyStatBytesAndRaiseFlag40BmagicMagic00800_801EF3F0(
    void) {
  u8 *work;
  u8 *work2;
  u8 *published;
  u32 slot;
  u32 offset;
  u32 word3C;
  u8 selector;
  u8 i;

  g_battle_work[8] = D_801EB4E0[8];
  work = g_battle_work;
  *(u32 *)&work[0x34] = *(u32 *)&D_801EB4E0[0x34];
  *(u32 *)&work[0x38] = *(u32 *)&D_801EB4E0[0x38];
  word3C = *(u32 *)&D_801EB4E0[0x3C];
  work[0xB] = 0;
  work = g_battle_work;
  *(u32 *)&work[0x3C] = word3C;
  work[9] = 0;
  g_battle_work[1] += 1;
  func_800AEF98(0xC, 2);
  for (i = 0; i < 3; i++) {
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
    published = g_battle_work;
    D_801EC331[offset] = 3;
    D_801EC336[offset] = 1;
    D_801EC332[offset] = 0;
    D_801EC335[offset] = 0x1F;
    D_801EC33B[offset] = i;
    work2 = g_battle_work;
    *(u32 *)&D_801EC3A4[offset] = (u32)published;
    work2[0xB] += 1;
  }
  g_battle_work[0x27] = (u8)func_800AFB48(D_801EB4E0);
  g_battle_work[0x28] = D_801EB4E0[0x28];
  g_battle_work[0x24] = D_801EB4E0[0x24];
  func_800AF8F0(g_battle_work);
  D_801EB4E0[0] |= 0x40;
}
