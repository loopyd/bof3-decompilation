#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Eight-byte pattern entries based at 0x801D393C inside this payload's data
 * textbin run at 0x801D3918. The unsigned byte at 0x801D40AC selects one of the
 * three entries the run holds (0x801D393C, 0x801D3944 and 0x801D394C); the code
 * pointer runs of this overlay begin at 0x801D3954, so the field quad ends at
 * 0x801D3953. The base pair is the screen-space origin the emitted records are
 * placed relative to, the signed halfword bounds both walks of this body and the
 * unsigned halfword is halved into the row-window step. */
typedef struct PatternGridEntry {
  u16 baseX; /* 0x00 */
  u16 baseY; /* 0x02 */
  s16 bound; /* 0x04 */
  u16 step;  /* 0x06 */
} PatternGridEntry;

extern PatternGridEntry D_801D393C[]; /* @source 0x801D393C @kind data */

/* Four-halfword projected screen pair: the resident projector 0x801AFF04
 * writes the halfword pair at +0x00/+0x02 and the trailing two halfwords stay
 * unwritten, the layout the exact lift
 * drawTexturedRibbonAlongWorkSpanWorld03Area13213_801F3268 declares for the
 * same call. */
typedef struct PatternGridScreen {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} PatternGridScreen;

/* Pattern-cell pointer of this payload's data blob: it walks the unsigned
 * halfwords of a cell run two bytes per grid cell and the run is only reached
 * through this cell, so its base stays address-anchored. The generated
 * undefined_syms_auto.txt row already names it. */
extern u8* D_801D40B0; /* @source 0x801D40B0 @kind data */

/* Write cursor of this payload's data blob into the shared ten-byte effect
 * record array: every emitted record advances it by 0xA and the record offsets
 * this body writes are the colour halfword at +0x2 and the position halfwords
 * at +0x4 and +0x6 of the record layout the exact lift at 0x801D34F0 owns. */
extern u8* D_801D40B4; /* @source 0x801D40B4 @kind data */

/* Live effect-record count of this payload's data blob: the exact lift at
 * 0x801D34F0 walked the same ten-byte records up to this halfword, and this body
 * adds one per emitted record. */
extern u16 D_801D40A8; /* @source 0x801D40A8 @kind data */

/* Pattern-entry index of this payload's data blob, part of the same textbin run
 * and runtime-filled. */
extern u8 D_801D40AC; /* @source 0x801D40AC @kind data */

/* Resident projection setup at 0x801AFE18: it fills the 0x20-byte scratch block
 * this body passes in $a0 and returns nothing. Resolved from this target's
 * generated undefined-function rows. */
void func_801AFE18(void* arg0);

/* Resident projector at 0x801AFF04: it projects the three-word point given in
 * $a0 into the halfword pair at the second argument, the pair the exact lifts of
 * the world and scenario families read back from the same call. Resolved from
 * this target's generated undefined-function rows. */
void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801D2474
 * @behavior Emitting step handler of this overlay, entry 2 of both code-pointer
 *           runs at 0x801D3954 and 0x801D397C, which the dispatchers 0x801D2170
 *           and 0x801D2DEC index with work byte +0x1. Through the scratchpad
 *           cell 0x1F800044 it fills the 0x20-byte projection scratch with the
 *           resident helper 0x801AFE18, copies the three words at +0x34, +0x38
 *           and +0x3C of the object behind work pointer word +0x4C into a point
 *           of its own and projects that point through 0x801AFF04 into the
 *           screen pair at frame +0x20. It then reads the pattern entry the
 *           payload byte 0x801D40AC selects from 0x801D393C, takes half of that
 *           entry's step halfword times the work counter byte at +0x9 as the
 *           starting cell index, and while that index stays under the same step
 *           times the incremented counter it walks the pattern-cell pointer
 *           0x801D40B0 over the entry's bound-by-bound grid, two bytes per cell.
 *           Every nonzero cell appends one ten-byte effect record at the cursor
 *           0x801D40B4: the cell is the record colour halfword at +0x2, the
 *           record position halfwords at +0x4 and +0x6 are the projected screen
 *           pair less the entry base pair plus the column and the masked cell
 *           index, the live count 0x801D40A8 is raised by one and the cursor
 *           advances by 0xA. The body finally increments work byte +0x9 and,
 *           when the stored value reads 2, advances work byte +0x1 to select the
 *           next step of the run. Every cell value, count and index is re-read
 *           per use because an emitted record may alias the published cells; the
 *           0x50-byte frame holds the 0x10-byte outgoing argument area, the
 *           point at +0x10, the screen pair at +0x20, the projection scratch at
 *           +0x28 and $ra at +0x48.
 *
 * Name evidence: the only in-image references to 0x801D2474 are the word at
 * 0x801D395C, word 2 of the ten-entry run based at 0x801D3954, and the word at
 * 0x801D3984, word 2 of the eleven-entry run based at 0x801D397C; no table and
 * no script word names this address. The body is an emitter, not a drawer: the
 * records it appends carry the colour halfword at +0x2 and the position
 * halfwords at +0x4 and +0x6 of the ten-byte layout the exact lift at 0x801D34F0
 * already owned, and the count halfword 0x801D40A8 it raises was the bound that
 * lift walked. Its last two blocks are the work byte +0x9 increment and the work
 * byte +0x1 advance the names of the surrounding step handlers follow.
 *
 * Shape evidence: the span at 0x801D30AC is byte-identical to this one over all
 * 146 words, so the payload holds a second copy of the body and neither copy had
 * a C seed; no other payload out of the 396 in out/binaries contains the span.
 * The scratchpad cell spelling, the 0x20-byte projection scratch and the
 * projector call pair follow the exact lifts of this overlay at 0x801D1A60 and
 * 0x801D106C. The two walks are a while over the row window and an inner for
 * over the entry bound; the inner for needs no pre-test of its own because the
 * bound it tests is the one the outer while already carries.
 * @status partial
 * @match 81.51
 * @residual The pattern-entry index. This candidate reproduces the span's 146
 *           instructions and its 584 bytes, and 119 instructions match in place;
 *           the first difference is instruction 23, where the original loads
 *           byte 0x801D40AC into $a0 and this candidate loads it into $a1. The
 *           original therefore reuses $a0 as the entry-base value of the second
 *           position store, which frees $a1 to hold the live count 0x801D40A8
 *           and lets that load issue before the colour store, while this
 *           candidate has to keep $a1 for the index, takes $v0 for both base
 *           values and issues the count load after the colour store. Every
 *           remaining difference is inside that block and follows from the one
 *           register choice, which the allocator reaches before any scheduling
 *           decision. Declaration order, the count local's type, the screen
 *           declarations, the emit statement order, the entry-pointer and index
 *           local forms, and both loop spellings were measured and did not move
 *           the allocation.
 *
 * Matching note: the block was measured first as a row loop guarded by its own
 * bound test over an inner for, which emitted the guard twice (101/146 at 584
 * bytes); as a do-while under that guard (106/148 at 592 bytes); and with the
 * pattern cell kept in a local pointer, which spent a saved register and grew
 * the frame (80/151 at 604 bytes). The while-over-for form above is what
 * reproduces the single guard and the exact size.
 */
void emitProjectedPatternGridEffectRecordsAndAdvanceWorkByte1WhenWorkByte9Reaches2ScenarioSce15ef100_801D2474(
    void) {
  s32 point[4];
  PatternGridScreen screen;
  u8 scratch[0x20];
  u8* record;
  s32 cellIndex;
  u8 column;
  u8 count;
  u8 counter;

  func_801AFE18(scratch);
  point[0] = *(s32*)(*(u32*)(D_1F800044 + 0x4C) + 0x34);
  point[1] = *(s32*)(*(u32*)(D_1F800044 + 0x4C) + 0x38);
  point[2] = *(s32*)(*(u32*)(D_1F800044 + 0x4C) + 0x3C);
  func_801AFF04(point, &screen);

  count = D_1F800044[9];
  cellIndex = ((s16)D_801D393C[D_801D40AC].step >> 1) * count;
  while ((u8)cellIndex <
         ((s16)D_801D393C[D_801D40AC].step >> 1) * (D_1F800044[9] + 1)) {
    for (column = 0u; column < D_801D393C[D_801D40AC].bound; column++) {
      if (*(u16*)D_801D40B0 != 0) {
        record = D_801D40B4;
        *(u16*)(record + 2) = *(u16*)D_801D40B0;
        D_801D40A8 = D_801D40A8 + 1;
        *(u16*)(record + 4) = screen.x - D_801D393C[D_801D40AC].baseX + column;
        D_801D40B4 = record + 0xA;
        *(u16*)(record + 6) =
            screen.y - D_801D393C[D_801D40AC].baseY + (u8)cellIndex;
      }
      D_801D40B0 = D_801D40B0 + 2;
    }
    cellIndex++;
  }
  counter = D_1F800044[9] + 1;
  D_1F800044[9] = counter;
  if (counter == 2) {
    D_1F800044[1] = D_1F800044[1] + 1;
  }
}
