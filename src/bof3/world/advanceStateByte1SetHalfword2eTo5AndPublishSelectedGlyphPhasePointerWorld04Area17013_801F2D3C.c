#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. This target's
 * map binds it, and the original re-reads the cell for the second access group
 * instead of keeping the pointer live, so the cell is named through the map
 * rather than through a fixed-address accessor. */
extern u8* D_1F800044;

/* Row phases of the overlay glyph banner, one byte per glyph, inside this
 * overlay's own payload. The exact draw routine at 0x801F31DC reads the same
 * three bytes as the starting row phase of the three glyph columns. */
extern u8 D_801F5990; /* @source 0x801F5990 @kind data */

/* @source 0x801F2D3C
 * @behavior Handler of the overlay dispatch table at 0x801F57BC. It reads the
 *           scratchpad work record from the pointer cell 0x1F800044, adds one
 *           to the record dispatch state byte at offset 0x01 (the byte the
 *           dispatcher at 0x801F2CF8 scales by four and indexes into that
 *           pointer run) and stores it back, then re-reads the pointer cell and
 *           stores 5 into the record halfword at 0x2E. It finally reads the
 *           record byte at 0x06, which selects one of the three glyphs of the
 *           banner, and stores the address of that glyph's row phase -
 *           0x801F5990 plus the selected byte - into the record word at 0x4C.
 *           The pad handler at 0x801F2D80 loads that word, moves the phase it
 *           points at between 0 and 8 and copies it back to 0x801F5990 plus the
 *           same selected byte, so the word publishes an editable pointer to the
 *           phase of the glyph the record has selected. Takes no arguments,
 *           calls nothing, returns nothing and keeps no frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Register shape: one local re-read from the pointer cell for both access
 * groups measured 13/17 instructions, the first difference being the first
 * word (`lui v1` against `lui a0`), because a single variable takes one color
 * for both groups. Reading the cell into a separate local per access group
 * gives the two temporaries their own colors and closed the diff. The
 * direct-cell spelling `D_1F800044[1] += 1;` for the first group measured the
 * same 17/17, so the two locals are a readable equivalent with equal bytes.
 */
void advanceStateByte1SetHalfword2eTo5AndPublishSelectedGlyphPhasePointerWorld04Area17013_801F2D3C(
    void) {
  u8* step;
  u8* record;
  u8* phase;

  step = D_1F800044;
  step[1] += 1;
  record = D_1F800044;
  *(u16*)(record + 0x2E) = 5;
  phase = (u8*)&D_801F5990 + record[6];
  *(u8**)(record + 0x4C) = phase;
}
