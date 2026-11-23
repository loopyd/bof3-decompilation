#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The body re-reads the cell with a fresh `lui` / `lw` pair
 * at every site - the byte 0xB clear, the byte 9 store, the byte 1 increment,
 * the published pointer word of each claimed slot, the two byte 0xB increments
 * and the slot byte 0x08 copy - because a store through the pointer it supplied
 * may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Action-slot pool at 0x801EC330 in main-executable RAM, outside this overlay's
 * image, whose 0x78-byte stride the allocated slot index scales. The original
 * reaches the copied entry through one base computed from D_801EC330 but
 * reaches each field it stamps as `lui $at,%hi(ADDR)` + `addu $at,$at,offset` +
 * `sb|sw %lo(ADDR)($at)`, so every stamped field keeps its own flat symbol. The
 * roles of the stamped bytes are not established here, so they stay
 * address-anchored. */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC335[]; /* @source 0x801EC335 @kind data */
extern u8 D_801EC336[]; /* @source 0x801EC336 @kind data */
extern u8 D_801EC338[]; /* @source 0x801EC338 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC359[]; /* @source 0x801EC359 @kind data */

/* Pointer word 0x74 of the same slot store, where this body publishes the
 * scratchpad work-object pointer. */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Enemy work record array at 0x801EB630 in main-executable RAM, scaled by the
 * 0x118-byte enemy stride; the acting battler selector minus three indexes it.
 * Only its 0x74-byte head is copied, and byte 0x801EB720 is that head's byte
 * 0xF0, the per-record byte this body tests against 0x29. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */
extern u8 D_801EB720[]; /* @source 0x801EB720 @kind data */

/* Shared battle work-record pointer cell at 0x801EB4E0 in main-executable RAM.
 * Only its byte 8 is read here, after a store that may alias the cell. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-RAM byte at 0x801462EA, tested before the closing slot is
 * claimed. */
extern u8 D_801462EA; /* @source 0x801462EA @kind data */

/* Acting battler selector at 0x80146374; party records are indexed 0 to 2 and
 * enemy records by selector minus three. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Shared main-RAM byte at 0x80145988, set to 1 on every path. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Action-slot allocator of the shared main executable at 0x801E590C. This body
 * claims with class 1 and mode 0x4B and uses the zero-extended low byte of the
 * answer as the slot index; it never tests a no-free-entry code. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* The 0x74-byte work head this body copies out of an enemy record: 29 words,
 * which the original moves as seven 16-byte chunks plus one trailing word. */
typedef struct BmagicMagic13703QueuedRecordHead {
  u32 words[29];
} BmagicMagic13703QueuedRecordHead;

/* @source 0x801EEC7C
 * @behavior Work byte 1 state 0 of this overlay: entry 0 of the three-word code
 *           pointer run at 0x801EEC04 (0x801EEC7C, 0x801EEEFC, 0x801EEF64),
 *           which the dispatcher at 0x801EEC1C copies into its own frame and
 *           calls through the scratchpad work object's byte 1, so no jal in the
 *           payload names this body. It clears work byte 0xB, sets work byte 9
 *           to 0x18 and increments work byte 1 at byte width, which advances
 *           that dispatcher to entry 1. It then claims two action slots through
 *           the shared allocator func_801E590C(1, 0x4B), takes the
 *           zero-extended low byte of each answer as the index of the
 *           0x78-byte-stride slot store at 0x801EC330 without testing a failure
 *           code, publishes the scratchpad work-object pointer read from the
 *           cell at 0x1F800044 into slot word 0x74 of both entries, stamps slot
 *           byte 0x01 with 0 in the first and 1 in the second, copies byte 8 of
 *           the shared battle work record reached through the pointer cell
 *           0x801EB4E0 into slot byte 0x08 of the first, and increments work
 *           byte 0xB once per claimed slot. It then copies the 0x100 unsigned
 *           halfwords of the table at 0x80036C00 into the table at 0x8003AC00
 *           with bit 15 set on every entry and stores 1 into the shared byte
 *           0x80145988. When the shared byte 0x801462EA is non-zero and the
 *           acting enemy record's byte 0xF0 - 0x801EB720, the byte
 *           (0x80146374 - 3) * 0x118 into the records based at 0x801EB630 -
 *           equals 0x29, it claims one more slot the same way, copies the
 *           0x74-byte work head at 0x801EB630 of that record into the entry and
 *           stamps slot bytes 0x01 = 2, 0x06 = 1, 0x05 = 0x4B, 0x02 = 0,
 *           0x09 = 0 and 0x29 = 3. The work-object cell and the two record
 *           cells are re-read after every store that may alias them, so each
 *           site keeps its own `lui` / `lw` pair. It takes no arguments and
 *           returns nothing; the 0x18-byte frame keeps only $ra at 0x10 across
 *           the three calls. The roles of the two halfword tables, of the
 *           stamped slot bytes, of the allocator's mode 0x4B and of the tested
 *           byte 0x29 stay unestablished, so they keep their literals and
 *           address anchors.
 * @status partial
 * @match 94.38
 * @residual Allocation and scheduling: 151/160 instructions at 0x280 bytes.
 *           The first difference is +0x144: the enemy selector used by the
 *           guard occupies $a0 rather than $v1. After the third allocation,
 *           the slot-stride calculation follows the selector load instead of
 *           preceding it. Memory accesses, calls and branch topology agree.
 *
 * Separate locals for the two published-pointer samples improved 121/160 to
 * 151/160 without changing size or memory events. GCC 2.7.2 local-alloc.c
 * admits single-block pseudos that die once, which motivated separating
 * these real samples; their exact RTL allocation path remains untraced. Splitting the tested enemy
 * selector from the post-call sample fixed the guard but regressed the tail
 * (148/160, 640 bytes), so it was not retained. Composing that split with an
 * early third-claim offset used only for the copy destination regressed to
 * 110/160 at 640 bytes and was also reverted.
 *
 * Representation notes from the shape probes measured on this span: with a
 * `u8 slot` local and a named `offset` reused at all three claim sites the
 * product stayed live across the copy loop and lost the original's three
 * instruction re-derivation of `slot * 0x78` (89/160, 640 bytes); taking the
 * enemy selector into a local before the 0x801462EA test instead of inside its
 * arm moved the -3 out of the symbol displacement (91/160, 640); a `u16 index`
 * re-masks the copy counter the original keeps mask-free (124/160, 656);
 * reading the record cell after the published store kept a load-delay nop the
 * original fills with that store (115/161); `u32 slot` with `(u8)` at the call
 * plus `slot * 0x78u` re-derived at the closing claim's copy destination and
 * its six stamps reproduced that re-derivation and reached this candidate's
 * size (121/160, 640); and the direct `(u32)g_battle_work` stores (116/160,
 * 632), a single shared work-pointer local (116/160, 632) and cursor locals
 * over the two halfword tables (124/160, 632) each dropped a memory event of
 * the original, so none stayed. A statement order that loads the second slot's
 * published pointer after its 0x801EC331 store measured 123/160 at 640 bytes
 * but emits that load where the original does not, so this candidate keeps the
 * load before the store, which is the order the original's own schedule shows.
 */
void stateSlotsActingHead(
    void) {
  s32 enemy;
  u32 slot;
  u32 offset;
  u8 *published;
  u8 *publishedSecond;
  u8 *queued;
  u8 *rec;
  u32 index;

  g_battle_work[0xB] = 0;
  g_battle_work[9] = 0x18;
  g_battle_work[1]++;

  slot = (u8)func_801E590C(1u, 0x4Bu);
  offset = slot * 0x78u;
  published = g_battle_work;
  D_801EC331[offset] = 0;
  rec = D_801EB4E0;
  *(u32 *)&D_801EC3A4[offset] = (u32)published;
  D_801EC338[offset] = rec[8];
  g_battle_work[0xB]++;

  slot = (u8)func_801E590C(1u, 0x4Bu);
  offset = slot * 0x78u;
  publishedSecond = g_battle_work;
  D_801EC331[offset] = 1;
  queued = g_battle_work;
  *(u32 *)&D_801EC3A4[offset] = (u32)publishedSecond;
  queued[0xB]++;

  for (index = 0; index < 0x100; index++) {
    PSX_PTR(u16, 0x8003AC00u)[index] = PSX_PTR(u16, 0x80036C00u)[index] | 0x8000;
  }
  D_80145988 = 1;

  if (D_801462EA != 0) {
    enemy = D_80146374;
    enemy -= 3;
    if (D_801EB720[enemy * 0x118] == 0x29) {
      slot = (u8)func_801E590C(1u, 0x4Bu);
      enemy = D_80146374;
      enemy -= 3;
      *(BmagicMagic13703QueuedRecordHead *)(D_801EC330 + slot * 0x78u) =
          *(BmagicMagic13703QueuedRecordHead *)(D_801EB630 +
                                                enemy * 0x118);
      D_801EC331[slot * 0x78u] = 2;
      D_801EC336[slot * 0x78u] = 1;
      D_801EC335[slot * 0x78u] = 75;
      D_801EC332[slot * 0x78u] = 0;
      D_801EC339[slot * 0x78u] = 0;
      D_801EC359[slot * 0x78u] = 3;
    }
  }
}
