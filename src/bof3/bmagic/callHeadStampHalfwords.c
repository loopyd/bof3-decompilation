#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, outside this overlay's
 * image. This target's map binds no name there, so the cell is reached through
 * its address-anchored name, the spelling this target's exact siblings
 * decrementGlobalWorkByte9AndAdvanceWorkByte2WhenHelperReady
 * BmagicMagic15403_801EF648 and
 * callAEF98With4And0ClearRecordFlag40RaiseBattleTargetFlag10AndAdvanceWorkByte1
 * WhenWorkByte9ZeroBmagicMagic15403_801EEFE0 bind the same cell with. The
 * original re-materialises the cell address at every access, because a store
 * through the pointer it supplied may alias the cell, so the body loads the cell
 * once for the published slot word and again for each of the two byte seeds and
 * the work byte 1 increment. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer cell at 0x801EB4E0 in main-executable RAM,
 * outside this overlay's image, bound by this target's generated
 * undefined_syms_auto.txt row. The original loads the record base once and
 * raises bit 0x40 of its byte 0, the role this target's exact sibling
 * callAEF98With4And0ClearRecordFlag40RaiseBattleTargetFlag10AndAdvanceWorkByte1
 * WhenWorkByte9ZeroBmagicMagic15403_801EEFE0 records for the same mask. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler selector of the main executable at 0x80146374, outside this
 * overlay's image: party records are indexed 0 to 2 and enemy records by
 * selector minus three, so the unsigned compare below splits the two record
 * arrays. The original loads it at byte width and tests it with sltiu 3. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Party battler record array at 0x80145E90, in main-executable RAM outside this
 * overlay's image, scaled by the 0x140-byte party stride. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Enemy battler record array at 0x801EB630, in the same main-executable region,
 * scaled by the 0x118-byte enemy stride; the acting battler selector minus three
 * indexes it. */
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

/* Shared main-RAM byte at 0x80145988, outside this overlay's image, stored as 1
 * through its own lui/sb pair after the halfword band copy. Other lifted bodies
 * in this tree describe it as a stage serial, but nothing in this target
 * establishes that role, so the name keeps the address anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Shared main-executable event dispatcher at 0x800AEF98, outside this overlay's
 * image and bound by this target's generated undefined_funcs_auto.txt row. The
 * original passes an event selector in the first argument and a value in the
 * second and uses no result, which is the prototype this target's exact sibling
 * callAEF98With4And0ClearRecordFlag40RaiseBattleTargetFlag10AndAdvanceWorkByte1
 * WhenWorkByte9ZeroBmagicMagic15403_801EEFE0 declares for the same call. */
extern void func_800AEF98(s32 event, s32 value); /* @source 0x800AEF98 @kind function */

/* Shared battle slot allocator at 0x801E590C, outside this overlay's image and
 * bound by this target's generated undefined_funcs_auto.txt row. The original
 * passes a mode word and a sub-mode and uses the low byte of the result as the
 * queued-slot index, which is the prototype and the truncation the exact
 * sibling seedWorkObjectClearBytesBAnd9DispatchEvent8ClaimSlot55CopyActingBattler
 * RecordHeadStampSlotBytesPublishWorkPointerCopyTwo15HalfwordBandsSet80145988
 * CopyRecordStatsAndRaiseFlag40BmagicMagic05603_801EEC94 declares for the same
 * call. */
extern u32 func_801E590C(u32 mode0, u32 mode1); /* @source 0x801E590C @kind function */

/* The 0x74-byte record head this body moves from the selected battler record
 * into the allocated slot entry. The original moves it as one block, 28 words in
 * a four-word loop plus the trailing word, which is the expansion of one 29-word
 * structure assignment; that remainder sits after the arm join because both arms
 * end in the same word copy. */
typedef struct BmagicMagic15403QueuedRecordHead {
  u32 words[29];
} BmagicMagic15403QueuedRecordHead;

/* @source 0x801EEC90
 * @behavior Entry 0 of the four-entry work byte 1 handler run at 0x801EEC04
 *           (0x801EEC90, 0x801EEE94, 0x801EEFE0, 0x801EF060), a run that lives
 *           in the overlay's own table words and that the exact sibling
 *           dispatcher dispatchWorkByte1Handler copies
 *           into its 0x28-byte frame and invokes with the work record's unsigned
 *           byte 1 scaled by four, so no jal in this payload names this address:
 *           an overlay dispatch entry, not a called function. It first
 *           dispatches event 0xC with the value 2 through the shared
 *           main-executable dispatcher func_800AEF98, then claims a queued slot
 *           through func_801E590C(1, 0x52) and uses the low byte of the result as
 *           the index into the 0x78-byte-stride slot store at 0x801EC330. It
 *           copies the acting battler's 0x74-byte record head into that slot:
 *           selectors of the shared byte 0x80146374 below three copy from the
 *           0x140-byte party records at 0x80145E90, and higher selectors from the
 *           0x118-byte enemy records at 0x801EB630 indexed by selector minus
 *           three. The claimed entry then receives byte +0x01 set to 1, byte
 *           +0x06 set to 1, byte +0x02 cleared, byte +0x05 set to 0x52 and word
 *           +0x74 loaded with the scratch work pointer read from the cell at
 *           0x1F800044. The body copies the sixteen unsigned halfwords of the
 *           table at 0x80036C00 into the table at 0x8003AC00 unchanged, testing
 *           a byte counter that starts at zero against 0x10 at the loop bottom.
 *           It then stores 1 into the shared main-RAM byte 0x80145988, raises
 *           bit 0x40 of byte 0 of the shared battle work record at 0x801EB4E0,
 *           seeds work bytes 0xB and 9 with 1 through two further reads of the
 *           pointer cell and increments work byte 1 through a third, advancing
 *           the work byte 1 state to run entry 1 (0x801EEE94). It takes no
 *           arguments, returns nothing and writes no other state; the 0x18-byte
 *           frame keeps only $ra at 0x10.
 *           Statement order, operand forms, offset-local spelling and
 *           declaration style follow the exact lift
 *           seedWorkObjectClearBytesBAnd9DispatchEvent8ClaimSlot55CopyActing
 *           BattlerRecordHeadStampSlotBytesPublishWorkPointerCopyTwo15Halfword
 *           BandsSet80145988CopyRecordStatsAndRaiseFlag40BmagicMagic05603_801EEC94,
 *           whose two-arm 0x74-byte record copy, slot stamp block, halfword band
 *           copy and closing record-flag raise carry the same instruction
 *           sequences; that sibling differs in its call targets and immediates,
 *           so it is shape evidence and not a byte-identical clone seed. This
 *           span's own bytes supplied the event 0xC with value 2, the slot mode
 *           0x52, the stamp literals 1, 1, 0 and 0x52, the single sixteen-halfword
 *           band copied unchanged from 0x80036C00 to 0x8003AC00, and the closing
 *           work byte seeds 1 and 1 followed by the work byte 1 increment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callHeadStampHalfwords(
    void) {
  u8 *published;
  u32 offset;
  u32 i;
  u32 slot;
  u8 selector;
  u16 *dst;
  u16 *src;

  func_800AEF98(0xC, 2);
  slot = (u8)func_801E590C(1, 0x52);
  selector = D_80146374;
  if (selector < 3) {
    BmagicMagic15403QueuedRecordHead *src2 =
        (BmagicMagic15403QueuedRecordHead *)&D_80145E90[selector * 0x140];
    BmagicMagic15403QueuedRecordHead *dst2 =
        (BmagicMagic15403QueuedRecordHead *)&D_801EC330[slot * 0x78];
    *dst2 = *src2;
  } else {
    BmagicMagic15403QueuedRecordHead *src2 =
        (BmagicMagic15403QueuedRecordHead *)&D_801EB630[(selector - 3) * 0x118];
    BmagicMagic15403QueuedRecordHead *dst2 =
        (BmagicMagic15403QueuedRecordHead *)&D_801EC330[slot * 0x78];
    *dst2 = *src2;
  }
  offset = slot * 0x78;
  published = D_1F800044;
  D_801EC331[offset] = 1;
  D_801EC336[offset] = 1;
  D_801EC332[offset] = 0;
  D_801EC335[offset] = 0x52;
  *(u32 *)&D_801EC3A4[offset] = (u32)published;
  i = 0;
  dst = PSX_PTR(u16, 0x8003AC00u);
  src = PSX_PTR(u16, 0x80036C00u);
  do {
    *dst++ = *src++;
    i++;
  } while (i < 0x10);
  D_80145988 = 1;
  D_801EB4E0[0] |= 0x40;
  D_1F800044[0x0B] = 1;
  D_1F800044[9] = 1;
  D_1F800044[1]++;
}
