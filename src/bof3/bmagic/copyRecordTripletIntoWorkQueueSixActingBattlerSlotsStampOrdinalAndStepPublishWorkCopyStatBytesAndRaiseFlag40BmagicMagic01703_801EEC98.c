#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named fixed address, the
 * form this overlay's exact siblings use, so the assembler folds the constant
 * into each access (lui $vN,0x1F80 / lw $vN,0x44($vN)). The original
 * re-materializes the cell once per store group, and this body holds four
 * separate reads: before the first byte store, before the 0x34/0x38/0x3C word
 * group, before work byte 9 is cleared, and before work byte 1 is raised. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so this lift needs no target-map row. The body reads
 * the record twice, once for the entry byte and the 0x34/0x38/0x3C word triplet
 * and once per tail statement, and raises bit 0x40 of record byte 0 at the end,
 * the same flag store this overlay's exact lift
 * invokeHelperThenClearRecordFlag40WhenWorkByteBZeroThenAdvanceWorkByte1Bmagic
 * Magic01703_801EEF68 performs with the opposite mask. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler selector of the main executable, re-read once per loop pass
 * because the slot allocator call may write it. Party slots are 0, 1 and 2 and
 * enemy panel slots are 3 and above, so the unsigned compare against 3 below
 * splits the two record arrays; the exact sibling
 * addWord0CToWord34AndWord10ToWord38And4ToTintTripletThenDecrementWorkByte9
 * AndOnZeroDecrementGlobalWorkByteBOrClearFlag20ResetWorkAdvanceWorkByte2Bmagic
 * Magic01703_801EF460 reads the same byte with the same split. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Local party record array at 0x80145E90, in main executable RAM outside this
 * overlay image, scaled by the 0x140-byte party stride. The 0x74-byte head the
 * body copies out of it is the record image the exact battle lift
 * copySelectedWorkToQueuedSlot copies into the same slot store from the full
 * record base. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Enemy record array at 0x801EB630, in main executable RAM outside this overlay
 * image, scaled by the 0x118-byte enemy stride; the acting battler selector
 * minus three indexes it, as in the exact sibling
 * addWord0CToWord34AndWord10ToWord38And4ToTintTripletThenDecrementWorkByte9
 * AndOnZeroDecrementGlobalWorkByteBOrClearFlag20ResetWorkAdvanceWorkByte2Bmagic
 * Magic01703_801EF460. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */

/* Queued slot store at 0x801EC330 in main executable RAM, whose 0x78-byte
 * stride this body scales the allocated index by to reach the copy destination.
 * The original reaches each stamped field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,offset` + `sb|sw %lo(ADDR)($at)`, so every addressed field keeps its
 * own flat symbol rather than one shared record base: bytes +0x01, +0x02,
 * +0x05, +0x06, +0x09 and +0x0B and the pointer word at +0x74. */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC335[]; /* @source 0x801EC335 @kind data */
extern u8 D_801EC336[]; /* @source 0x801EC336 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC33B[]; /* @source 0x801EC33B @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared slot-4 event dispatcher of the battle overlays at 0x800AEF98, outside
 * this overlay image and owned by the battle targets; the first argument is a
 * byte-sized event selector and the second a byte-sized value, and no result is
 * used. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is added here. */
void func_800AEF98(s32 event, s32 value);

/* Slot allocator at 0x801E590C, outside this overlay image: it claims the first
 * entry of the 0x30-entry queued slot store whose byte +0x00 has bit 0 clear,
 * sets that bit, stores the two mode bytes at +0x06 and +0x05 and returns the
 * entry index, or 0xFF when every entry is already active. */
u32 func_801E590C(u32 arg0, u32 arg1);

/* Main-executable helpers at 0x800AFB48 and 0x800AF8F0, outside this overlay
 * image. Both read the record they are handed at bytes 0x28, 0x27 and 0x24, the
 * three bytes this body publishes into the work object immediately before and
 * after them; the first returns a byte that this call site stores, the second
 * returns nothing. Neither is lifted in the tree, so both keep their
 * address-qualified names. */
u32 func_800AFB48(u8 *record);
void func_800AF8F0(u8 *work);

/* Main-executable helper at 0x8015D71C; this overlay passes the selector that
 * re-arms the work byte 9 countdown. The exact sibling
 * incrementWorkByte9To16ThenSeedStepPairFromWorkByteBAddPublishedRecordPosition
 * CopyWord3CSetVerticalStepInvokeHelperAt101RearmByte9AndAdvanceWorkByte2Bmagic
 * Magic01703_801EF300 declares the same prototype. */
void func_8015D71C(s32 arg0);

/* The 0x74-byte record head this body moves from a battler record into a queued
 * slot entry; the original moves it as one block, 28 words in a four-word loop
 * plus the trailing word, which is the expansion of one 29-word structure
 * assignment. */
typedef struct Magic01703SlotBody {
  u32 words[29];
} Magic01703SlotBody;

/* @source 0x801EEC98
 * @behavior Work byte 1 state 0 of this overlay: entry 0 of the three-word
 *           handler table at 0x801EEC04 (0x801EEC98, 0x801EEF68, 0x801EEFD8),
 *           which func_801EEC38 copies into its frame and indexes with the
 *           scratchpad work object's byte 1, so the body runs while work byte 1
 *           is 0 and raises that byte to 1 itself. It first copies the shared
 *           battle work record's byte 8 and its word triplet 0x34/0x38/0x3C
 *           into the work object, clears work bytes 0xB and 9, increments work
 *           byte 1 and invokes the shared dispatcher at 0x800AEF98 with the
 *           event selector 0xC and the value 0. It then makes six passes over
 *           the queued slot store: each pass claims a slot through
 *           func_801E590C(1, 0x21), selects the acting battler's record by the
 *           selector byte at 0x80146374 - party records at 0x80145E90 with the
 *           0x140-byte party stride for selectors 0 to 2, enemy records at
 *           0x801EB630 with the 0x118-byte enemy stride for selector 3 and above
 *           indexed by selector minus 3 - and copies that record's 0x74-byte
 *           head into the claimed slot entry. Each pass then stamps the entry
 *           bytes +0x06 with 1, +0x05 with 0x21, +0x01 and +0x02 with 0, +0x0B
 *           with the pass ordinal and +0x09 with the pass ordinal masked by 0xFE
 *           and raised by one, publishes the work-object pointer in the entry's
 *           word +0x74 and increments the work object's byte 0xB. After the six
 *           passes it stores the byte func_800AFB48 returns for the shared record
 *           at work byte 0x27, copies record bytes 0x28 and 0x24 into the same
 *           work bytes, calls func_800AF8F0 on the work object, raises bit 0x40
 *           of record byte 0 and calls func_8015D71C with 0x100. The 0x20-byte
 *           frame keeps $ra at 0x18 and $s0/$s1 at 0x10/0x14 across the calls,
 *           and the three pointer-cell loads that precede the frame are the
 *           entry reads the scheduler hoisted above the stack adjustment. Takes
 *           no arguments and returns nothing. The roles of the stamped entry
 *           bytes and of record bytes 8, 0x24, 0x34, 0x38, 0x3C, 0x27 and 0x28
 *           are not established, so the name describes the measured effect and
 *           those fields stay offset-anchored.
 * @status partial
 * @match 95.00
 * @residual 171/180 instructions, 720 bytes. The first difference is +0x0034:
 *           the entry triplet uses $a1 rather than $a2 for the scratchpad work
 *           pointer; its last word store uses the next cell read, and the call
 *           setup schedules differently.
 *           Both record-copy loops, slot stamps, work-pointer publication and
 *           the tail match.
 */
void copyRecordTripletIntoWorkQueueSixActingBattlerSlotsStampOrdinalAndStepPublishWorkCopyStatBytesAndRaiseFlag40BmagicMagic01703_801EEC98(
    void) {
  u8 *work;
  u8 *work2;
  u32 slot;
  u32 offset;
  u32 word3C;
  u8 sel;
  u8 i;

  D_1F800044[8] = D_801EB4E0[8];
  *(u32 *)&D_1F800044[0x34] = *(u32 *)&D_801EB4E0[0x34];
  *(u32 *)&D_1F800044[0x38] = *(u32 *)&D_801EB4E0[0x38];
  word3C = *(u32 *)&D_801EB4E0[0x3C];
  D_1F800044[0xB] = 0;
  *(u32 *)&D_1F800044[0x3C] = word3C;
  D_1F800044[9] = 0;
  D_1F800044[1] += 1;
  func_800AEF98(0xC, 0);
  for (i = 0; i < 6; i++) {
    slot = func_801E590C(1, 0x21);
    sel = D_80146374;
    if (sel < 3) {
      Magic01703SlotBody *src =
          (Magic01703SlotBody *)&D_80145E90[sel * 0x140];
      *(Magic01703SlotBody *)&D_801EC330[(u8)slot * 0x78] = *(Magic01703SlotBody *)src;
    } else {
      Magic01703SlotBody *src =
          (Magic01703SlotBody *)&D_801EB630[(sel - 3) * 0x118];
      *(Magic01703SlotBody *)&D_801EC330[(u8)slot * 0x78] = *(Magic01703SlotBody *)src;
    }
    offset = (u8)slot * 0x78;
    work = D_1F800044;
    D_801EC336[offset] = 1;
    D_801EC335[offset] = 0x21;
    D_801EC331[offset] = 0;
    D_801EC332[offset] = 0;
    D_801EC33B[offset] = i;
    D_801EC339[offset] = (u8)((i & 0xFE) + 1);
    work2 = D_1F800044;
    *(u32 *)&D_801EC3A4[offset] = (u32)work;
    work2[0xB] += 1;
  }
  D_1F800044[0x27] = (u8)func_800AFB48(D_801EB4E0);
  D_1F800044[0x28] = D_801EB4E0[0x28];
  D_1F800044[0x24] = D_801EB4E0[0x24];
  func_800AF8F0(D_1F800044);
  D_801EB4E0[0] |= 0x40;
  func_8015D71C(0x100);
}
