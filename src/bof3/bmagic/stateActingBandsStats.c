#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. The body re-reads the cell with a fresh `lui` / `lw`
 * pair at every site - the byte 0xB clear, the byte 9 clear, the work byte 1
 * increment, the published pointer word, the work byte 0xB count, the three
 * published battle-record bytes and the closing helper argument - because a
 * store through the pointer it supplied may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer cell at 0x801EB4E0 in main-executable RAM,
 * outside this overlay's image. This body passes it to the spawning helper at
 * 0x800AFB48, copies its bytes 0x28 and 0x24 into the work object and raises
 * bit 0x40 of its byte 0, each after a store that may alias the cell. It
 * belongs to the battle targets, so the field roles stay address-anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler selector of the main executable at 0x80146374, outside this
 * overlay's image: party records are indexed 0 to 2 and enemy records by
 * selector minus three, so the unsigned compare below splits the two record
 * arrays. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Party battler record array at 0x80145E90, in main-executable RAM outside this
 * overlay's image, scaled by the 0x140-byte party stride. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Enemy battler record array at 0x801EB630, in the same main-executable region,
 * scaled by the 0x118-byte enemy stride; the acting battler selector minus
 * three indexes it. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */

/* Queued-slot store at 0x801EC330 in main-executable RAM, whose 0x78-byte
 * stride the allocated slot index scales to reach the copy destination. The
 * original reaches each stamped field as `lui $at,%hi(ADDR)` +
 * `addu $at,$at,offset` + `sb|sw %lo(ADDR)($at)` with one shared scaled index,
 * so each addressed field keeps its own flat symbol: the bytes at +0x01, +0x02,
 * +0x05 and +0x06 and the pointer word at +0x74. */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC335[]; /* @source 0x801EC335 @kind data */
extern u8 D_801EC336[]; /* @source 0x801EC336 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Channel mask tables of the main executable, outside this overlay's image: the
 * two leading halfwords at 0x8003AC00 and 0x8003AC20 are cleared and the
 * fifteen halfwords that follow each of them receive bit 15 set from the tables
 * at 0x80036C02 and 0x80036C22. Both destination bases are addressed as their
 * own symbols, while the two working pointers are absolute-address casts in the
 * reading loops. */
extern u16 D_8003AC00; /* @source 0x8003AC00 @kind data */
extern u16 D_8003AC20; /* @source 0x8003AC20 @kind data */

/* Shared battle flag byte at 0x80145988, outside this overlay's image, set to
 * one by this body after the channel masks are armed. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Shared work-object seeding helper at 0x800AFD9C, outside this overlay's
 * image and owned by the battle targets; this body only calls it, with no
 * argument set at the site. */
extern void func_800AFD9C(void); /* @source 0x800AFD9C */

/* Shared event dispatcher at 0x800AEF98, outside this overlay's image: a
 * byte-sized event selector and a byte-sized value, no result used. */
extern void func_800AEF98(s32 event, s32 value); /* @source 0x800AEF98 */

/* Slot allocator at 0x801E590C, outside this overlay's image: it claims the
 * first entry of the queued-slot store whose byte +0x00 has bit 0 clear, sets
 * that bit and returns the entry index, or 0xFF when every entry is active.
 * This body truncates the answer to a byte and scales it by the 0x78-byte
 * stride without testing the failure code. */
extern u32 func_801E590C(u32 mode0, u32 mode1); /* @source 0x801E590C */

/* Spawning helper at 0x800AFB48 that consumes the shared battle work record and
 * returns the byte this body publishes into work byte 0x27. */
extern u32 func_800AFB48(u8 *record); /* @source 0x800AFB48 */

/* Closing helper at 0x800AF8F0 that consumes the seeded work object. */
extern void func_800AF8F0(u8 *work); /* @source 0x800AF8F0 */

/* The 0x74-byte record head this body moves from a battler record into the
 * claimed slot entry. The original moves it as one block, 28 words in a
 * four-word loop plus the trailing word, which is the expansion of one 29-word
 * structure assignment. */
typedef struct BmagicMagic05603QueuedRecordHead {
  u32 words[29];
} BmagicMagic05603QueuedRecordHead;

/* @source 0x801EEC94
 * @behavior Word 0 of the three-entry code pointer run at 0x801EEC04 (0x801EEC94,
 *           0x801EEF3C, 0x801EF17C), which the entry dispatcher
 *           dispatchWorkByte1HandlerBmagicMagic05603_801EEC34 copies into its own
 *           0x28-byte frame and invokes with the scratchpad work object byte 1
 *           scaled by four as index, so no jal in this payload names this body.
 *           It first calls the shared seeding helper at 0x800AFD9C, then clears
 *           work bytes 0xB and 9, increments work byte 1 and invokes the shared
 *           event dispatcher at 0x800AEF98 with selector 8 and value 0, the
 *           increment filling the call delay slot. It claims a slot through
 *           func_801E590C(1, 0x55) and copies the acting battler's 0x74-byte
 *           record head into that slot of the 0x78-byte-stride store at
 *           0x801EC330: selectors of the shared byte 0x80146374 below three copy
 *           from the 0x140-byte party records at 0x80145E90, and higher
 *           selectors from the 0x118-byte enemy records at 0x801EB630 indexed by
 *           selector minus three. The claimed entry then receives byte +0x01
 *           set to 3, byte +0x06 set to 1, byte +0x02 cleared, byte +0x05 set
 *           to 0x55 and word +0x74 loaded with the work pointer, and work byte
 *           0xB counts the claim. The body arms the channel masks next: it
 *           clears the two leading halfwords at 0x8003AC00 and 0x8003AC20 and
 *           copies fifteen halfwords into each of the two following bands from
 *           0x80036C02 and 0x80036C22 with bit 15 set, counting from one to
 *           0x0F. It then publishes 1 into the shared byte 0x80145988, stores
 *           the byte the helper at 0x800AFB48 returns for the shared battle work
 *           record into work byte 0x27, copies that record's bytes 0x28 and
 *           0x24 into the work object, invokes the closing helper at 0x800AF8F0
 *           with the work object and raises bit 0x40 of the shared record byte
 *           0. It takes no arguments, returns nothing and writes no other state;
 *           its 0x18-byte frame keeps only the return address at 0x10, and every
 *           store group re-reads a cell because a store may alias it.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation notes from the shape probe measured before this body matched:
 * declaring the allocator result as `u8 slot` let the compiler defer the
 * truncation, so the prologue copied the return register and masked again at
 * each scaled site (158/174 instructions, 680 -> 696 bytes, first difference
 * `move t1,v0` where the original masks once at 0x801EECF8); truncating at the
 * assignment with `(u8)` and scaling the `u32` local at each site masked once
 * and reused that register, which measured 170/170 and 680 bytes. That winning
 * probe also moved the head pointers from function scope to the top of each
 * arm; the two changes were measured together, so neither is attested on its
 * own.
 */
void stateActingBandsStats(
    void) {
  u8 *work;
  u8 *published;
  u8 *queued;
  u32 offset;
  u32 i;
  u32 slot;
  u8 selector;
  u16 *dst;
  u16 *src;

  func_800AFD9C();
  g_battle_work[0x0B] = 0;
  g_battle_work[9] = 0;
  work = g_battle_work;
  work[1]++;
  func_800AEF98(8, 0);
  slot = (u8)func_801E590C(1, 0x55);
  selector = D_80146374;
  if (selector < 3) {
    BmagicMagic05603QueuedRecordHead *src2 =
        (BmagicMagic05603QueuedRecordHead *)&D_80145E90[selector * 0x140];
    BmagicMagic05603QueuedRecordHead *dst2 =
        (BmagicMagic05603QueuedRecordHead *)&D_801EC330[slot * 0x78];
    *dst2 = *src2;
  } else {
    BmagicMagic05603QueuedRecordHead *src2 =
        (BmagicMagic05603QueuedRecordHead *)&D_801EB630[(selector - 3) * 0x118];
    BmagicMagic05603QueuedRecordHead *dst2 =
        (BmagicMagic05603QueuedRecordHead *)&D_801EC330[slot * 0x78];
    *dst2 = *src2;
  }
  offset = slot * 0x78;
  published = g_battle_work;
  D_801EC331[offset] = 3;
  D_801EC336[offset] = 1;
  D_801EC332[offset] = 0;
  D_801EC335[offset] = 0x55;
  queued = g_battle_work;
  *(u32 *)&D_801EC3A4[offset] = (u32)published;
  queued[0x0B]++;
  i = 1;
  dst = PSX_PTR(u16, 0x8003AC02u);
  src = PSX_PTR(u16, 0x80036C02u);
  do {
    dst[0] = src[0] | 0x8000;
    dst[0x10] = src[0x10] | 0x8000;
    src++;
    i++;
    dst++;
  } while (i < 0x10);
  D_8003AC00 = 0;
  D_8003AC20 = 0;
  D_80145988 = 1;
  g_battle_work[0x27] = (u8)func_800AFB48(D_801EB4E0);
  g_battle_work[0x28] = D_801EB4E0[0x28];
  g_battle_work[0x24] = D_801EB4E0[0x24];
  func_800AF8F0(g_battle_work);
  D_801EB4E0[0] |= 0x40;
}
