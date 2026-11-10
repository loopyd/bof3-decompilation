#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work like the other lifts of this target declare it. The body
 * re-reads the cell at every site - the arm that publishes the pointer into the
 * claimed slot, the count increment after that arm and the byte 0x1 increment -
 * because the stores through the loaded pointer may alias the cell, so the
 * original re-materializes the address with its own `lui` / `lw` pair at each
 * read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Byte table at 0x800B6BF0 in executable RAM, outside this overlay image. The
 * work object's byte 0x4 selects one entry with an indexed byte load and that
 * entry is the first argument of the selection helper below, so the table is
 * addressed through the one symbol the original's `lui $at` / `addu` / `lbu`
 * triple names. */
extern u8 D_800B6BF0[]; /* @source 0x800B6BF0 @kind data */

/* Shared battle target byte at 0x80146384: the battle target selector that the
 * other lifts of this payload pass to the battle helpers
 * (src/bof3/bmagic/decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordAndAdvanceWorkByte1OnZeroBmagicMagic01000_801EEE20.c).
 * Here it is the second argument of the selection helper. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Action-slot store at 0x801EC330 in main RAM: 0x78-byte stride, the pool the
 * sibling queueActionSlotBmagicMagic1730003 lift documents. The original reaches
 * each field through its own flat symbol - `lui $at,%hi(SYM)`, `addu $at,$at,
 * index*0x78`, store `%lo(SYM)($at)` - with the scaled index in one register
 * shared by all four stores, so every addressed field keeps its own
 * address-anchored name: the kind byte at +0x04, the bytes at +0x09 and +0x0A and
 * the work-object pointer word at +0x74. */
extern u8 D_801EC334[]; /* @source 0x801EC334 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC33A[]; /* @source 0x801EC33A @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Slot allocator of the battle image at 0x801E590C, outside this overlay and
 * bound through this target's generated undefined_funcs_auto.txt. It returns the
 * index of the claimed action slot, which both arms truncate to a byte before
 * scaling it by the 0x78-byte stride. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Battle selection helper of 0x800AED5C in executable RAM, outside this overlay
 * and bound through this target's generated undefined_funcs_auto.txt. It takes
 * the table entry selected by the work object's byte 0x4 plus the shared battle
 * target byte, publishes that selection into its own cells and reports a byte
 * result; this body keeps only that truncated result. */
extern u32 func_800AED5C(u32 arg0, u32 arg1); /* @source 0x800AED5C */

/* @source 0x801EF1AC
 * @behavior Per-frame step of an overlay work-object handler. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 and returns at
 *           once while the object's byte 0xB is nonzero. Otherwise it calls the
 *           selection helper at 0x800AED5C with the byte table entry that the
 *           object's byte 0x4 selects and the shared battle target byte at
 *           0x80146384, and truncates the answer to a byte. Both arms then claim
 *           an action slot through the allocator at 0x801E590C with the
 *           arguments 1 and 0x48, truncate the returned index to a byte and
 *           scale it by the 0x78-byte slot stride. The arm taken when the
 *           selection byte is nonzero publishes the re-read work-object pointer
 *           into the slot word +0x74 first, then writes the slot kind byte +0x04
 *           with the object's byte 0x4 plus 4, the slot byte +0x09 with one and
 *           the slot byte +0x0A with zero. The arm taken when the selection byte
 *           is zero writes the same four fields in the opposite order - kind
 *           byte 8, byte +0x09 one, byte +0x0A zero, then the work-object
 *           pointer - and the pointer it publishes is the value of the single
 *           cell read the arm made before those stores. The tail then re-reads
 *           the pointer cell for each of its two increments: work byte 0xB and
 *           work byte 0x1, both at byte width through lbu / addiu 1 / sb. The
 *           0x18-byte frame holds $ra only. It takes no arguments, returns
 *           nothing and reads or writes no other state. No byte-identical twin
 *           of these 0x154 bytes exists in any emi payload of this repository,
 *           so the seed is written from this span's own bytes; the same-address
 *           0x801EF1AC of bmagic/magic038/03 is a longer, unlifted neighbour and
 *           names nothing for this body.
 * @status partial
 * @match 83.53
 * @residual Fourteen of the 85 instructions differ and every differing word is
 *           a register spelling or a block offset inside the two claim arms.
 *           The original keeps the scaled slot index in $v0 and the
 *           work-object pointer in $v1; this candidate keeps the product in $a1
 *           and the pointer in $a0, so the four address triples and the two
 *           constant loads of each arm differ only by register. The original
 *           additionally hoists the tail's first pointer-cell read above the
 *           then arm's `j` onto the join block, which this candidate leaves at
 *           the join. The two arms' store order, byte widths, constants, the
 *           0x18-byte frame and the 0x154-byte size already agree. The levers
 *           already measured are the index local's width and cast site
 *           (u8/u16/u32, `(u8)` at the assignment, `& 0xFF`, chained), the
 *           offset local against the inline repeated subscript that this
 *           candidate keeps, and the placement of each arm's cell read against
 *           the product; the remaining difference is a register tie, so the
 *           next rung is compiler allocation evidence rather than another
 *           spelling.
 */
void queueActionSlotSetByte4Byte9AndByteAAndPublishWorkPointerThenCountWorkByteBAndIncrementWorkByte1BmagicMagic08803_801EF1AC(void) {
  u8 *work;
  u8 *record;
  u32 slot;

  work = g_battle_work;
  if (work[0xB] != 0) {
    return;
  }
  if ((u8)func_800AED5C(D_800B6BF0[work[4]], D_80146384) != 0) {
    slot = (u8)func_801E590C(1, 0x48);
    record = g_battle_work;
    *(u32 *)&D_801EC3A4[slot * 0x78u] = (u32)record;
    D_801EC339[slot * 0x78u] = 1;
    D_801EC33A[slot * 0x78u] = 0;
    D_801EC334[slot * 0x78u] = record[4] + 4;
  } else {
    slot = (u8)func_801E590C(1, 0x48);
    record = g_battle_work;
    D_801EC334[slot * 0x78u] = 8;
    D_801EC339[slot * 0x78u] = 1;
    D_801EC33A[slot * 0x78u] = 0;
    *(u32 *)&D_801EC3A4[slot * 0x78u] = (u32)record;
  }
  g_battle_work[0xB]++;
  g_battle_work[1]++;
}
