#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. The body re-reads the cell with a fresh `lui` / `lw`
 * pair at every site - the byte 0xB clear, the byte 9 store, the byte 1
 * increment, the published pointer word of each claimed slot and the two byte
 * 0xB increments - because a store through the pointer it supplied may alias
 * the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer cell at 0x801EB4E0 in main-executable RAM,
 * outside this overlay's image. Only byte 8 of the record it points at is read
 * here, after a store that may alias the cell, and the field role stays
 * address-anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Quoted-slot pool at 0x801EC330 in main-executable RAM, whose 0x78-byte stride
 * the allocated slot index scales. The original reaches the copied entry
 * through one base computed from D_801EC330 but reaches each field it stamps as
 * `lui $at,%hi(ADDR)` + `addu $at,$at,offset` + `sb|sw %lo(ADDR)($at)` with one
 * shared scaled index, so every stamped field keeps its own flat symbol. The
 * roles of the stamped bytes are not established here, so they stay
 * address-anchored. */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC335[]; /* @source 0x801EC335 @kind data */
extern u8 D_801EC336[]; /* @source 0x801EC336 @kind data */
extern u8 D_801EC338[]; /* @source 0x801EC338 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */

/* Pointer word 0x74 of the same slot store, where this body publishes the
 * scratchpad work-object pointer. */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Enemy work record array at 0x801EB630 in main-executable RAM, scaled by the
 * 0x118-byte enemy stride; the acting battler selector minus three indexes it.
 * Only its 0x74-byte head is copied, into a slot the body claims when the
 * per-record byte 0x801EB720 tests equal to 0x29. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */
extern u8 D_801EB720[]; /* @source 0x801EB720 @kind data */

/* Shared main-RAM byte at 0x801462EA, tested before the closing slot is
 * claimed. */
extern u8 D_801462EA; /* @source 0x801462EA @kind data */

/* Acting battler selector at 0x80146374; party records are indexed 0 to 2 and
 * enemy records by selector minus three, so the unsigned `lbu` before the
 * subtraction splits the two record arrays. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Shared main-RAM byte at 0x80145988, set to 1 on every path. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Slot allocator of the shared main executable at 0x801E590C. This body claims
 * with class 1 and mode 0x13 three times, keeps the zero-extended low byte of
 * each answer as the slot index and never tests a no-free-entry code. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C @kind function */

/* The 0x74-byte record head this body copies out of an enemy record: 29 words,
 * which the original moves as seven 16-byte chunks plus one trailing word. */
typedef struct BmagicMagic06703QueuedRecordHead {
  u32 words[29];
} BmagicMagic06703QueuedRecordHead;

/* @source 0x801EEC94
 * @behavior Work byte 1 state 0 of this overlay: entry 0 of the five-word code
 *           pointer run at 0x801EEC04 (0x801EEC94, 0x801EEF3C, 0x801EEFA4,
 *           0x801EF010, 0x801EF284), which the dispatcher at 0x801EEC24 copies
 *           into its own frame and calls through the scratchpad work object's
 *           byte 1, so no jal in the payload names this body. It clears work
 *           byte 0xB, sets work byte 9 to 0x18 and increments work byte 1 at
 *           byte width, which advances that dispatcher to entry 1. It then
 *           claims two action slots through the shared allocator
 *           func_801E590C(1, 0x13), takes the zero-extended low byte of each
 *           answer as the index of the 0x78-byte-stride slot store at
 *           0x801EC330 without testing a failure code, publishes the scratchpad
 *           work-object pointer read from the cell at 0x1F800044 into slot word
 *           0x74 of both entries, stamps slot byte 0x01 with 0 in the first and
 *           1 in the second, copies byte 8 of the shared battle work record
 *           reached through the pointer cell 0x801EB4E0 into slot byte 0x08 of
 *           the first, and increments work byte 0xB once per claimed slot. It
 *           then copies the 0x100 unsigned halfwords of the table at 0x80033C00
 *           into the table at 0x80037C00 with bit 15 set on every entry, copies
 *           the two 16-halfword bands at 0x80036C00 and 0x80036C20 into
 *           0x8003AC00 and 0x8003AC20, and stores 1 into the shared byte
 *           0x80145988. When the shared byte 0x801462EA is non-zero and the
 *           acting enemy record's per-record byte - 0x801EB720, the byte
 *           (0x80146374 - 3) * 0x118 into the records based at 0x801EB630 -
 *           equals 0x29, it claims one more slot the same way, copies the
 *           0x74-byte record head of that enemy record into the entry and stamps
 *           slot bytes 0x01 = 3, 0x06 = 1, 0x02 = 0, 0x05 = 0x13 and 0x09 = 0.
 *           The work-object cell and the record cells are re-read after every
 *           store that may alias them, so each site keeps its own `lui` / `lw`
 *           pair. It takes no arguments and returns nothing; the 0x18-byte
 *           frame keeps only $ra at 0x10 across the three calls. The roles of
 *           the two halfword table pairs, of the stamped slot bytes, of the
 *           allocator's mode 0x13 and of the tested byte 0x29 stay
 *           unestablished, so they keep their literals and address anchors.
 * @status partial
 * @match 98.82
 * @residual Scheduling: 168/170 instructions at the original 0x2A8 bytes, no
 *           frame, memory or call difference. The first difference is +0x1C4:
 *           the acting selector load (`lui` / `lbu` of 0x80146374) is scheduled
 *           before the three-instruction `slot * 0x78` expansion that the
 *           original runs between the mask and that load, so the same eight
 *           instructions appear in one swapped pair order. Allocation agrees at
 *           that site ($a3 masked slot, $a0 selector, $v1 destination base,
 *           $a2 source base, $t0 destination cursor, $t1 source end).
 *
 * Statement order, operand forms, offset spelling and declaration style follow
 * the near-identical body setWorkByte9To18ClaimTwoSlotsCopyHalfwordTableWithBit
 * 15SetAndWhenActingRecordByteF0Is29ClaimSlotCopyWorkHeadBmagicMagic13703_801EE
 * C7C, which applies the same allocator class, slot stamps and enemy-record copy
 * at its own literals. This span's mode literal 0x13, its halfword table pair
 * (0x80033C00 to 0x80037C00 with bit 15 set), its plain two-band copy at
 * 0x80036C00 into 0x8003AC00 / 0x8003AC20 and its closing stamp set (0x01 = 3,
 * 0x05 = 0x13, no byte 0x29 stamp) were read from this span's own bytes, and one
 * payload scan proved this 0x2A8-byte range appears in no other shipped
 * payload, so the sibling is shape evidence and not a clone.
 *
 * Shape probes measured before this candidate (all at the original 0x2A8 bytes
 * unless noted): one shared `dst` / `src` cursor pair for both halfword loops
 * with one shared counter, which is the form the sibling partial uses, measured
 * 118/170 at the same first difference +0x44; separate counters for the two
 * loops 120; block-scoped `slot` / `offset` per claim 112; an unmasked `slot`
 * with the mask folded into each product 121; a named `offset` at the closing
 * claim 130; the selection guard written inline 153 at 676 bytes; and the
 * selector shared between the guard and the closing claim 162. Splitting the
 * two loop cursor pairs (0x80033C00/0x80037C00 and 0x8003AC00/0x80036C00) into
 * their own locals moved the first difference to +0x17C (161), and giving the
 * closing claim its own `src2` / `dst2` record-head pointers with a separate
 * post-call selector local moved it to +0x1C4 (168), which is this candidate.
 */
void stateBandsActingHead(
    void) {
  s32 enemy;
  u32 slot;
  u32 offset;
  u8 *published;
  u8 *publishedSecond;
  u8 *queued;
  u8 *rec;
  u16 *dstA;
  u16 *srcA;
  u16 *dstB;
  u16 *srcB;
  u32 index;

  g_battle_work[0xB] = 0;
  g_battle_work[9] = 0x18;
  g_battle_work[1]++;

  slot = (u8)func_801E590C(1u, 0x13u);
  offset = slot * 0x78u;
  published = g_battle_work;
  D_801EC331[offset] = 0;
  rec = D_801EB4E0;
  *(u32 *)&D_801EC3A4[offset] = (u32)published;
  D_801EC338[offset] = rec[8];
  g_battle_work[0xB]++;

  slot = (u8)func_801E590C(1u, 0x13u);
  offset = slot * 0x78u;
  publishedSecond = g_battle_work;
  D_801EC331[offset] = 1;
  queued = g_battle_work;
  *(u32 *)&D_801EC3A4[offset] = (u32)publishedSecond;
  queued[0xB]++;

  index = 0;
  dstA = PSX_PTR(u16, 0x80037C00u);
  srcA = PSX_PTR(u16, 0x80033C00u);
  do {
    dstA[0] = srcA[0] | 0x8000;
    srcA++;
    index++;
    dstA++;
  } while (index < 0x100);

  index = 0;
  dstB = PSX_PTR(u16, 0x8003AC00u);
  srcB = PSX_PTR(u16, 0x80036C00u);
  do {
    dstB[0] = srcB[0];
    index++;
    dstB[0x10] = srcB[0x10];
    srcB++;
    dstB++;
  } while (index < 0x10);

  D_80145988 = 1;

  if (D_801462EA != 0) {
    enemy = D_80146374;
    enemy -= 3;
    if (D_801EB720[enemy * 0x118] == 0x29) {
      BmagicMagic06703QueuedRecordHead *src2;
      BmagicMagic06703QueuedRecordHead *dst2;
      s32 acting;
      slot = (u8)func_801E590C(1u, 0x13u);
      acting = D_80146374;
      acting -= 3;
      src2 = (BmagicMagic06703QueuedRecordHead *)(D_801EB630 + acting * 0x118);
      dst2 = (BmagicMagic06703QueuedRecordHead *)(D_801EC330 + slot * 0x78u);
      *dst2 = *src2;
      D_801EC331[slot * 0x78u] = 3;
      D_801EC336[slot * 0x78u] = 1;
      D_801EC332[slot * 0x78u] = 0;
      D_801EC335[slot * 0x78u] = 0x13;
      D_801EC339[slot * 0x78u] = 0;
    }
  }
}
