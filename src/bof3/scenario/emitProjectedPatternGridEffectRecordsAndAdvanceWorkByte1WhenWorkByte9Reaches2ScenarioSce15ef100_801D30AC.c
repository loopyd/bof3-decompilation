#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Eight-byte pattern entries based at 0x801D393C inside this payload's data
 * textbin run at 0x801D3918. The unsigned byte at 0x801D40AC selects one of the
 * three entries the run holds (0x801D393C, 0x801D3944 and 0x801D394C). The base
 * pair is the screen-space origin the emitted records are placed relative to,
 * the signed halfword bounds the row walk of this body and the unsigned
 * halfword is halved into the cell-window step. */
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

/* @source 0x801D30AC
 * @behavior Emitting step handler of this overlay, word 7 of the eleven-entry
 *           code-pointer run at 0x801D397C that the dispatcher 0x801D2DEC
 *           indexes with work byte +0x1. Through the scratchpad cell 0x1F800044
 *           it fills the 0x20-byte projection scratch with the resident helper
 *           0x801AFE18, copies the three words at +0x34, +0x38 and +0x3C of the
 *           object behind work pointer word +0x4C into a point of its own and
 *           projects that point through 0x801AFF04 into the screen pair at frame
 *           +0x20. It then reads the pattern entry the payload byte 0x801D40AC
 *           selects from 0x801D393C, takes half of that entry's step halfword
 *           times the work counter byte at +0x9 as the starting cell index, and
 *           while that index stays under the same half-step times the
 *           incremented counter it walks the pattern-cell pointer 0x801D40B0
 *           over the entry's bound-by-bound grid, two bytes per cell. Every
 *           nonzero cell appends one ten-byte effect record at the cursor
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
 * Name evidence: the only word in this payload that addresses 0x801D30AC is
 * 0x801D3988-relative entry 0x801D3998, word 7 of the eleven-entry code-pointer
 * run based at 0x801D397C that the dispatcher at 0x801D2DEC indexes with work
 * byte +0x1; the ten-entry run at 0x801D3954 stops short of it, so no other run
 * and no script word names this address. The body is an emitter, not a drawer:
 * the records it appends carry the colour halfword at +0x2 and the position
 * halfwords at +0x4 and +0x6 of the ten-byte layout the exact lift at 0x801D34F0
 * already owned, and the count halfword 0x801D40A8 it raises was the bound that
 * lift walked. Its last two blocks are the work byte +0x9 increment and the work
 * byte +0x1 advance the names of the surrounding step handlers follow.
 *
 * Shape evidence: this span is byte-identical to the 0x248-byte span at
 * 0x801D2474 over all 146 words, so the payload holds a second copy of the body;
 * the clone candidate is that lift's body with this address's declarations and
 * no word of the payload addresses any part of the span except its run entry.
 * The two spans therefore share one reconstruction problem, and this candidate
 * starts from the best measured shape of that duplicate.
 * @status partial
 * @match 81.51
 * @residual The pattern-entry index. This candidate reproduces the span's 146
 *           instructions and its 584 bytes and 119 instructions match in place;
 *           the first difference is instruction 23, where the original loads
 *           byte 0x801D40AC into $a0 and this candidate loads it into $a1. The
 *           original therefore reuses $a0 for the entry base value of the second
 *           position store, which frees $a1 to hold the live count 0x801D40A8
 *           and lets that load issue before the colour store, while the count
 *           store fills the entry-base load slot; this candidate keeps $a1 for
 *           the index, spends $v0 on the count and issues it after the colour
 *           store. Measured in this lane: moving the count update ahead of the
 *           colour store - the order the original's own instruction sequence
 *           implies, since the count load precedes the colour store - does give
 *           the index $a0, but it rotates the record cursor to $a1, the row
 *           counter to $a2 and the cell index to $a3 and emits 148 instructions
 *           in 592 bytes at 110/146 from instruction 36; the same move ahead of
 *           the cursor read, and the u16-pointer, struct-record and cell-local
 *           spellings of the emit block, measured the same or one slot worse and
 *           were reverted. The remaining difference follows from that one
 *           allocation choice, which the allocator reaches before any scheduling
 *           decision.
 */
void emitProjectedPatternGridEffectRecordsAndAdvanceWorkByte1WhenWorkByte9Reaches2ScenarioSce15ef100_801D30AC(
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
