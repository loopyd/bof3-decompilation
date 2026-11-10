#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, spelled through this address-derived
 * symbol, the same named cell the exact lifts of this target already use. A named
 * cell symbol lets the assembler fold the constant address into each load
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)), and the original re-reads the cell after
 * every byte or word store made through the pointer it supplied, because such a
 * store may alias the cell object. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so this lift adds no target-map row. The body reads
 * the record in three groups: its byte 8 and word triplet 0x34/0x38/0x3C at the
 * head, once per queued slot to hand to the publish call, and once per published
 * byte at the tail, then raises bit 0x40 of record byte 0. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler selector of the main executable, re-read once per pass because
 * the slot allocator call may write it. Party slots are 0, 1 and 2 and enemy
 * panel slots are 3 and above, so the unsigned compare against 3 below splits
 * the two record arrays; the tail tests the same byte again after the publish
 * calls, so only a party selector takes the final helper call. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Local party record array at 0x80145E90, in main executable RAM outside this
 * overlay image, scaled by the 0x140-byte party stride. The 0x74-byte head the
 * body copies out of it is the record image the exact battle lift
 * copySelectedWorkToQueuedSlot copies into the same slot store from the full
 * record base. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Enemy record array at 0x801EB630, in main executable RAM outside this overlay
 * image, scaled by the 0x118-byte enemy stride; the acting battler selector minus
 * three indexes it. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */

/* Queued slot store at 0x801EC330 in main executable RAM, whose 0x78-byte stride
 * this body scales the allocated index by to reach the copy destination. The
 * original reaches each stamped field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,offset` + `sb|sw %lo(ADDR)($at)`, so every addressed field keeps its
 * own flat symbol rather than one shared record base: bytes +0x01, +0x02, +0x05,
 * +0x06, +0x09 and +0x0B and the pointer word at +0x74. */
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
 * entry of the queued slot store whose byte +0x00 has bit 0 clear, sets that bit,
 * stores the two mode bytes at +0x06 and +0x05 and returns the entry index, or
 * 0xFF when every entry is already active. */
u32 func_801E590C(u32 arg0, u32 arg1);

/* Main-executable helper at 0x800AFB48, outside this overlay image: it reads the
 * shared battle work record it is handed and returns the byte this call site
 * publishes into work byte 0x27. */
u32 func_800AFB48(u8 *record);

/* Main-executable helper at 0x800AF8F0, outside this overlay image; this call
 * site only hands it the work object and uses no result. */
void func_800AF8F0(u8 *work);

/* Shared battle helper at 0x800AFC98, outside this overlay image; this call site
 * passes the immediates 2 and 0 and uses no result. */
void func_800AFC98(s32 arg0, s32 arg1);

/* The 0x74-byte record head this body moves from a battler record into a queued
 * slot entry; the original moves it as one block, 28 words in a four-word loop
 * plus the trailing word, which is the expansion of one 29-word structure
 * assignment. */
typedef struct BmagicMagic02003SlotBody {
  u32 words[29];
} BmagicMagic02003SlotBody;

/* @source 0x801EECC0
 * @behavior Work byte 1 state 0 of this overlay: entry 0 of the six-word handler
 *           table at 0x801EEC04 (0x801EECC0, 0x801EF0F8, 0x801EF234, 0x801EF2A8,
 *           0x801EF31C, 0x801EF350), which
 *           dispatchWorkByte1HandlerBmagicMagic02003_801EEC48 copies into its
 *           frame and indexes with the scratchpad work object's byte 1, so the
 *           body runs while work byte 1 is 0 and raises that byte to 1 itself. It
 *           first copies the shared battle work record's byte 8 and word triplet
 *           0x34/0x38/0x3C into the work object, clears work bytes 0xB and 9,
 *           increments work byte 1 and invokes the shared dispatcher at
 *           0x800AEF98 with the event selector 0xC and the value 2. It then makes
 *           four passes over the queued slot store: each pass claims a slot
 *           through func_801E590C(1, 0x2E), selects the acting battler's record
 *           by the selector byte at 0x80146374 - party records at 0x80145E90 with
 *           the 0x140-byte party stride for selectors 0 to 2, enemy records at
 *           0x801EB630 with the 0x118-byte enemy stride for selector 3 and above
 *           indexed by selector minus 3 - and copies that record's 0x74-byte head
 *           into the claimed slot entry at 0x801EC330 scaled by the 0x78-byte
 *           slot stride. Each pass then stamps the entry bytes +0x05 with 0x2E,
 *           +0x01 and +0x06 with 1, +0x02 with 0, +0x09 with the pass ordinal
 *           xor 3 raised by one and +0x0B with the pass ordinal xor 3, publishes
 *           the work-object pointer in the entry word +0x74 and advances the
 *           loop. After the four passes one more slot is claimed and filled the
 *           same way, stamped +0x06 with 1, +0x01 and +0x02 with 0 and +0x05 with
 *           0x2E, with no ordinal bytes and no work byte 0xB increment. The body
 *           then stores the byte func_800AFB48 returns for the shared record at
 *           work byte 0x27, copies record bytes 0x28 and 0x24 into the same work
 *           bytes, calls func_800AF8F0 on the work object, calls func_800AFC98
 *           with 2 and 0 only when the acting battler selector is below 3, and
 *           raises bit 0x40 of record byte 0. The 0x20-byte frame keeps $ra at
 *           0x1C, $s2 at 0x18 and $s1/$s0 at 0x14/0x10 across the calls, and the
 *           two pointer-cell loads that precede the frame are the reads the
 *           scheduler hoisted above the stack adjustment. Takes no arguments and
 *           returns nothing. The roles of the stamped entry bytes, of the work
 *           bytes 0x24/0x27/0x28 and of work bytes 1, 9 and 0xB are not
 *           established, so the name describes the measured effect and those
 *           fields stay offset-anchored. Statement order, the record-head copy
 *           expansion and the field stamping form follow this family's sibling
 *           copyRecordTripletIntoWorkQueueSixActingBattlerSlotsStampOrdinalAndSt
 *           epPublishWorkCopyStatBytesAndRaiseFlag40BmagicMagic01703_801EEC98,
 *           which was still a 95 percent lift when this seed was written: it
 *           walks the same store sequence, the same 0x74-byte record image and
 *           the same record-selection split, with six passes and the immediates
 *           this span does not use. A byte search over every payload under
 *           out/binaries found no other copy of this 0x438-byte span, so the
 *           clone lever was declined. The span opens with addiu $sp,$sp,-0x20 at
 *           0x801EECD0, closes in the jr $ra at 0x801EF0F0 through that frame and
 *           holds six jal sites, so the data lever did not apply, and it sits
 *           between the reviewed boundaries at 0x801EEC48, whose span ends at
 *           this address, and 0x801EF0F8, so the carve lever did not apply
 *           either.
 * @status partial
 * @match 88.52
 * @residual 239/270 instructions, 1080 bytes, first difference at +0x0190. The
 *           whole prologue, the first claim and its record copy, the loop head
 *           and both post-loop arms match. The original issues the entry
 *           ordinal (xori $a0,$s0,3) and the loop increment before the
 *           slot-offset scaling and keeps the offset in $v0, the ordinal in $a0
 *           and the work pointer in $a1; this body scales the offset first and
 *           keeps the offset in $a0, the ordinal in $v1 and the work pointer in
 *           $a2, with $v0 carrying the 0x2E mode byte. The same permutation
 *           repeats in the fifth claim's stamp block, where the original also
 *           loads the record pointer before the +0x74 store. Every remaining
 *           difference is an instruction-order or register-choice permutation
 *           of the same instruction multiset: statement order, an ordinal local
 *           before the scaling, an ordinal local before the copy, a do-while
 *           loop with an explicit increment, the work pointer read before the
 *           scaling, and an inline scaled index each measured the same or lower
 *           without moving the first difference, so the residual is a
 *           scheduling and register-allocation choice, not a statement shape
 *           this source is missing.
 */
void seedWorkObjectFromBattleRecordThenCopyActingBattlerRecordsIntoFiveQueuedSlotsStampMirroredOrdinalsPublishRecordBytes24And27And28AndRaiseFlag40BmagicMagic02003_801EECC0(
    void) {
  u8 *work;
  u8 *record;
  u8 *work2;
  u32 slot;
  u32 offset;
  u32 word3C;
  u8 sel;
  u8 i;

  D_1F800044[8] = D_801EB4E0[8];
  work = D_1F800044;
  *(u32 *)&work[0x34] = *(u32 *)&D_801EB4E0[0x34];
  *(u32 *)&work[0x38] = *(u32 *)&D_801EB4E0[0x38];
  word3C = *(u32 *)&D_801EB4E0[0x3C];
  work[0xB] = 0;
  work2 = D_1F800044;
  *(u32 *)&work[0x3C] = word3C;
  work2[9] = 0;
  D_1F800044[1] += 1;
  func_800AEF98(0xC, 2);
  for (i = 0; i < 4; i++) {
    slot = func_801E590C(1, 0x2E);
    sel = D_80146374;
    if (sel < 3) {
      BmagicMagic02003SlotBody *src =
          (BmagicMagic02003SlotBody *)&D_80145E90[sel * 0x140];
      *(BmagicMagic02003SlotBody *)&D_801EC330[(u8)slot * 0x78] =
          *(BmagicMagic02003SlotBody *)src;
    } else {
      BmagicMagic02003SlotBody *src =
          (BmagicMagic02003SlotBody *)&D_801EB630[(sel - 3) * 0x118];
      *(BmagicMagic02003SlotBody *)&D_801EC330[(u8)slot * 0x78] =
          *(BmagicMagic02003SlotBody *)src;
    }
    offset = (u8)slot * 0x78;
    D_801EC335[offset] = 0x2E;
    D_801EC331[offset] = 1;
    D_801EC332[offset] = 0;
    D_801EC336[offset] = 1;
    D_801EC339[offset] = (i ^ 3) + 1;
    D_801EC33B[offset] = i ^ 3;
    *(u32 *)&D_801EC3A4[offset] = (u32)D_1F800044;
  }
  slot = func_801E590C(1, 0x2E);
  sel = D_80146374;
  if (sel < 3) {
    BmagicMagic02003SlotBody *src =
        (BmagicMagic02003SlotBody *)&D_80145E90[sel * 0x140];
    BmagicMagic02003SlotBody *dst =
        (BmagicMagic02003SlotBody *)&D_801EC330[(u8)slot * 0x78];
    *dst = *src;
  } else {
    BmagicMagic02003SlotBody *src =
        (BmagicMagic02003SlotBody *)&D_801EB630[(sel - 3) * 0x118];
    BmagicMagic02003SlotBody *dst =
        (BmagicMagic02003SlotBody *)&D_801EC330[(u8)slot * 0x78];
    *dst = *src;
  }
  offset = (u8)slot * 0x78;
  work = D_1F800044;
  D_801EC336[offset] = 1;
  D_801EC331[offset] = 0;
  D_801EC332[offset] = 0;
  D_801EC335[offset] = 0x2E;
  record = D_801EB4E0;
  *(u32 *)&D_801EC3A4[offset] = (u32)work;
  D_1F800044[0x27] = (u8)func_800AFB48(record);
  D_1F800044[0x28] = D_801EB4E0[0x28];
  D_1F800044[0x24] = D_801EB4E0[0x24];
  func_800AF8F0(D_1F800044);
  if (D_80146374 < 3) {
    func_800AFC98(2, 0);
  }
  D_801EB4E0[0] |= 0x40;
}
