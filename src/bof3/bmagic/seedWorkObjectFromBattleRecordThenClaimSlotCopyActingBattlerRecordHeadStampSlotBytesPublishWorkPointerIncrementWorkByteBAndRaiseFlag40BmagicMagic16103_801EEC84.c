#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, shared by this overlay's
 * handlers. The original re-materializes the cell address with its own
 * `lui` / `lw` pair before every read - the byte 8 copy, the word triplet copy,
 * the work byte 1 increment, the published pointer word and the work byte 0xB
 * increment - because a store through the pointer it supplied may alias the
 * cell. The body therefore reads the cell as a plain expression at each site;
 * the same declaration spells it this way in the exact sibling
 * dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkBytes0And2SetBmagicMagic16103_801EEF80
 * of this target. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer cell at 0x801EB4E0 in main-executable RAM,
 * outside this overlay's image. This body reads it three times: for byte 8, for
 * the word triplet 0x34/0x38/0x3C and for byte 0 at the tail, each after a
 * store that may alias the cell. This target's generated
 * undefined_syms_auto.txt already names the address, so no target-map row is
 * added and the field roles stay address-anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler selector of the main executable at 0x80146374, outside this
 * overlay's image: party records are indexed 0 to 2 and enemy records by
 * selector minus three, so the unsigned compare below splits the two record
 * arrays. This target's generated undefined_syms_auto.txt names the
 * address. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Party battler record array at 0x80145E90, in main-executable RAM outside
 * this overlay's image, scaled by the 0x140-byte party stride. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Enemy battler record array at 0x801EB630, in the same main-executable region,
 * scaled by the 0x118-byte enemy stride; the acting battler selector minus
 * three indexes it. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */

/* Queued-slot store at 0x801EC330 in main-executable RAM, whose 0x78-byte
 * stride this body scales the allocated slot index by to reach the copy
 * destination. The original reaches each stamped field as `lui $at,%hi(ADDR)` +
 * `addu $at,$at,offset` + `sb|sw %lo(ADDR)($at)` with one scaled index shared by
 * every store, so each addressed field keeps its own flat symbol: the bytes at
 * +0x01, +0x02, +0x05 and +0x06 and the pointer word at +0x74. */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC335[]; /* @source 0x801EC335 @kind data */
extern u8 D_801EC336[]; /* @source 0x801EC336 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared slot-4 event dispatcher of the battle overlays at 0x800AEF98, outside
 * this overlay's image and owned by the battle targets; this lift only calls
 * it. The prototype follows the call site: a byte-sized event selector and a
 * byte-sized value, with no result used. This target's generated
 * undefined_funcs_auto.txt names the address. */
extern void func_800AEF98(s32 event, s32 value); /* @source 0x800AEF98 */

/* Slot allocator at 0x801E590C, outside this overlay's image: it claims the
 * first entry of the queued-slot store whose byte +0x00 has bit 0 clear, sets
 * that bit and returns the entry index, or 0xFF when every entry is already
 * active. This body uses the zero-extended low byte of the answer as the index
 * it scales by the 0x78-byte stride and does not test the failure code. */
extern u32 func_801E590C(u32 mode0, u32 mode1); /* @source 0x801E590C */

/* The 0x74-byte record head this body moves from a battler record into the
 * claimed slot entry. The original moves it as one block, 28 words in a
 * four-word loop plus the trailing word, which is the expansion of one 29-word
 * structure assignment. */
typedef struct BmagicMagic16103QueuedRecordHead {
  u32 words[29];
} BmagicMagic16103QueuedRecordHead;

/* @source 0x801EEC84
 * @behavior Work byte 1 state 0 of this overlay: entry 0 of the three-entry code
 *           pointer run at 0x801EEC04 (0x801EEC84, 0x801EEE9C, 0x801EEF08),
 *           which the overlay entry dispatcher at 0x801EEC24 copies into its own
 *           frame and invokes with the scratchpad work-object byte 1 as index;
 *           no jal in the payload names this body. It first copies the shared
 *           battle work record's byte 8 and word triplet 0x34/0x38/0x3C from the
 *           pointer cell at 0x801EB4E0 into the work object, clearing work byte
 *           0xB between the last word read and that word's store, then
 *           increments work byte 1 at byte width and invokes the shared battle
 *           helper at 0x800AEF98 with the event selector 0xC and the value 2.
 *           It then claims one slot through the allocator at 0x801E590C with the
 *           identity bytes 1 and 0x54 and copies the acting battler's 0x74-byte
 *           record head into the claimed entry of the 0x78-byte-stride slot
 *           store at 0x801EC330. Selectors below 3 copy from the 0x140-byte
 *           party records at 0x80145E90 and other selectors from the
 *           0x118-byte enemy records at 0x801EB630 indexed by selector minus 3;
 *           the selector is the acting battler byte at 0x80146374 and the
 *           allocator's 0xFF failure code is not tested. It stamps the entry
 *           byte +0x06 with 1, bytes +0x01 and +0x02 with 0 and byte +0x05 with
 *           0x54, publishes the scratchpad work-object pointer in entry word
 *           +0x74, increments work byte 0xB and raises bit 0x40 of the shared
 *           work record's byte 0. It takes no arguments, returns nothing and
 *           reads or writes no other state; the 0x18-byte frame only keeps $ra
 *           at 0x10 across the two calls. The 0x218-byte body carries the same
 *           statement order, operand forms and offset spelling as the
 *           already-exact sibling
 *           seedWorkObjectFromBattleRecordThenClaimSlotCopyActingBattlerRecordHeadStampSlotBytesPublishWorkPointerRaiseFlag40AndInvokeHelperAt100BmagicMagic00800_801EED5C,
 *           with four differences read from this span's own bytes: the allocator
 *           and stamped-slot constants are 0x54 here against 0x1F there, this
 *           body has no work byte 0x9 clear and no trailing invocation of
 *           0x8015D71C, and it increments work byte 0xB after publishing the
 *           pointer. Event selector 0xC, value 2, the identity bytes 1 and 0x54
 *           and the record, slot and table field roles stay unestablished
 *           constants and offsets.
 *
 * Representation notes: the published pointer and the counted pointer are read
 * into separate locals. Publishing through the same local as the head's word
 * triplet measured 131/134 because the allocator then keeps that pointer in
 * $a1, where the original holds the published pointer in $a0, and publishing
 * through the head's record local measured 114/134 because it moves the head's
 * record pointer out of $v1. Raising the tail flag through the shared record
 * cell as an expression matched all 134 instructions and 536 bytes, as did
 * reloading it into the counted local.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkObjectFromBattleRecordThenClaimSlotCopyActingBattlerRecordHeadStampSlotBytesPublishWorkPointerIncrementWorkByteBAndRaiseFlag40BmagicMagic16103_801EEC84(
    void) {
  u8 *record;
  u8 *work;
  u8 *next;
  u8 *published;
  u8 *queued;
  u32 value;
  u32 slot;
  u32 offset;
  u8 selector;

  g_battle_work[8] = D_801EB4E0[8];
  record = D_801EB4E0;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
  value = *(u32 *)(record + 0x3C);
  work[0x0B] = 0;
  next = g_battle_work;
  *(u32 *)(work + 0x3C) = value;
  next[1]++;
  func_800AEF98(0xC, 2);
  slot = func_801E590C(1, 0x54);
  selector = D_80146374;
  if (selector < 3) {
    BmagicMagic16103QueuedRecordHead *src =
        (BmagicMagic16103QueuedRecordHead *)&D_80145E90[selector * 0x140];
    BmagicMagic16103QueuedRecordHead *dst =
        (BmagicMagic16103QueuedRecordHead *)&D_801EC330[(u8)slot * 0x78];
    *dst = *src;
  } else {
    BmagicMagic16103QueuedRecordHead *src =
        (BmagicMagic16103QueuedRecordHead *)&D_801EB630[(selector - 3) * 0x118];
    BmagicMagic16103QueuedRecordHead *dst =
        (BmagicMagic16103QueuedRecordHead *)&D_801EC330[(u8)slot * 0x78];
    *dst = *src;
  }
  offset = (u8)slot * 0x78;
  published = g_battle_work;
  D_801EC336[offset] = 1;
  D_801EC331[offset] = 0;
  D_801EC332[offset] = 0;
  D_801EC335[offset] = 0x54;
  queued = g_battle_work;
  *(u32 *)&D_801EC3A4[offset] = (u32)published;
  queued[0x0B]++;
  D_801EB4E0[0] |= 0x40;
}
