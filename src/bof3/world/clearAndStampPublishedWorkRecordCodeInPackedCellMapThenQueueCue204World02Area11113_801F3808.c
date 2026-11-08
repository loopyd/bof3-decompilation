#include "bof3/bof3.h"

/* 152-byte (0x98) work-record cell of the shared table based at 0x80147A58.
 * Only the stride is proven here: the original scales the distance between the
 * published record and that table by exactly 0x98, the same stride this
 * overlay's neighbouring steps use for these cells. */
typedef struct World02Area11113WorkRecord {
  u8 bytes[0x98];
} World02Area11113WorkRecord;

/* Pointer cell of the published 0x98-byte work record. */
extern World02Area11113WorkRecord* D_80146884; /* @source 0x80146884 @kind unknown */

/* Base of the shared 0x98-byte work-record table. */
extern World02Area11113WorkRecord D_80147A58[]; /* @source 0x80147A58 @kind bss */

/* Packed cell-code map inside this overlay's trailing data blob: two nibbles
 * per byte, four bytes per map column, so the original addresses it as
 * column * 4 + row / 2. This body rewrites the span, and each rewritten byte is
 * read twice - once for the nibble comparison and again for the stored value -
 * so the span is bound volatile. */
extern volatile u8 D_801F5C18[]; /* @source 0x801F5C18 @kind data */

/* Overlay cell draw: queues the four shared UI elements around the grid point
 * of a cell through the exe-resident queue at 0x801A4BC0. */
void func_801F4604(s16 row, s16 col, u32 size);

/* Shared cue dispatcher of the main executable. */
void func_8015DF18(u16 cue);

/* @source 0x801F3808
 * @behavior Area step that moves the cell code of the published 0x98-byte work
 *           record to the cell that record now occupies. It builds the
 *           one-based code of that record from its index in the shared table at
 *           0x80147A58 (the pointer published through 0x80146884 minus that
 *           base, 0x98-byte stride, plus one), halves the signed coordinates
 *           stored at bytes 0x3A and 0x36 of the work object published at the
 *           scratchpad pointer cell 0x1F800044 after subtracting their grid
 *           origins 0xB1 and 0xB, and draws that cell through the overlay draw
 *           at 0x801F4604 with size 0. It then walks the whole 7x7 map of
 *           packed nibbles at 0x801F5C18 (column * 4 + row / 2, low nibble on an
 *           odd row and the high nibble on an even one) and clears every nibble
 *           equal to that code, leaving the other nibble of the byte untouched,
 *           before stamping the code back into the record's own cell with an
 *           OR, and finally queues cue 0x204 through the shared dispatcher at
 *           0x8015DF18. Takes no arguments and returns nothing; the 0x20-byte
 *           frame keeps $ra, $s0, $s1 and $s2 across the two calls.
 *
 * Name evidence: 0x80147A58 is the 0x98-byte work-record table and 0x80146884
 *           the pointer cell that publishes the record this overlay's other
 *           steps in this payload already name, 0x801F5C18 is the packed cell
 *           map the selector and request steps of this payload resolve their
 *           slots from, 0x1F800044 is the scratchpad work-object pointer cell
 *           the same family reads, 0x801F4604 is this overlay's own cell draw
 *           (four 0x801A4BC0 element queues around the cell) and 0x8015DF18 the
 *           shared cue dispatcher. The code itself is only proven to be the
 *           record index plus one, so the name stays at the map operation.
 *
 * Shape: no byte-identical twin exists. The full 0x1D8-byte range was compared
 *           against every payload under out/binaries with no hit, not even on
 *           the leading 64 bytes, and the neighbouring steps at 0x801F35F4 and
 *           0x801F39E0 are same-target siblings: shape evidence, never clone
 *           eligibility. The span is one complete function (prologue, two
 *           calls, one `jr $ra` epilogue), so no carve applies, and it is code,
 *           not a data span. The body is seeded from the original bytes: the
 *           slot quotient follows the exact sibling at 0x801F3790 with this
 *           body's signed shift, the nibble walk matches the map indexing the
 *           siblings at 0x801F35F4/0x801F39E0 use, and the tail follows the
 *           same odd-row/low-nibble convention.
 * @status partial
 * @match 55.08
 * @residual 65 of 118 instructions, 468 of 472 bytes; first difference at
 *           0x801F3808 (index 0). The build hoists the scratchpad record read
 *           and the halved row above the record-index chain, so the leading
 *           eleven instructions are ordered differently even though the index
 *           chain itself, the 7x7 nibble walk and the cue tail reproduce. The
 *           two map rewrites also keep one address register where the original
 *           recomputes the base plus the halved row inside each arm.
 */
void clearAndStampPublishedWorkRecordCodeInPackedCellMapThenQueueCue204World02Area11113_801F3808(
    void) {
  u8* work;
  volatile u8* map;
  s32 rowHalf;
  s32 colHalf;
  s32 index;
  u32 code;
  u8 col;
  u8 row;
  u8 nibbles;

  index = (s32)((((u32)((u8*)D_80146884 - (u8*)D_80147A58)) * 0x286BCA1Bu)) >> 3;
  code = index + 1;
  work = SPAD_PTR_SLOT(u8, 0x44);
  rowHalf = (*(s16*)(work + 0x36) - 0xB) / 2;
  colHalf = (*(s16*)(work + 0x3A) - 0xB1) / 2;
  func_801F4604(rowHalf, colHalf, 0);
  for (col = 0; col < 7; col++) {
    map = D_801F5C18 + (col & 0xFF) * 4;
    for (row = 0; row < 7; row++) {
      nibbles = D_801F5C18[(col & 0xFF) * 4 + (row >> 1)];
      if ((nibbles & 0xF) == (u8)code) {
        map[row >> 1] = map[row >> 1] & 0xF0;
      } else if ((nibbles >> 4) == (u8)code) {
        map[row >> 1] = map[row >> 1] & 0xF;
      }
    }
  }
  if (rowHalf & 1) {
    D_801F5C18[(s16)colHalf * 4 + (s16)rowHalf / 2] |= code;
  } else {
    D_801F5C18[(s16)colHalf * 4 + (s16)rowHalf / 2] |= code << 4;
  }
  func_8015DF18(0x204);
}
