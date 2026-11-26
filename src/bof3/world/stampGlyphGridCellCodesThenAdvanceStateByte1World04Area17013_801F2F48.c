#include "bof3/bof3.h"

/* Stamp routine of this overlay: writes the 7x9 banner glyph grid into the map
 * cell codes and queues one shared UI element per cell. */
void stampElementsAttachTexture(
    void);

/* @source 0x801F2F48
 * @behavior Handler at slot 2 of the overlay dispatch table at 0x801F57BC (the
 *           pointer word at 0x801F57C4): it stamps the 7x9 glyph grid of the
 *           overlay banner into the map cell codes by calling the stamp routine
 *           at 0x801F3994 with no arguments, then reads the scratchpad work
 *           record from the pointer cell 0x1F800044 and adds one to the record
 *           dispatch state byte at offset 0x01, the byte the dispatcher at
 *           0x801F2CF8 scales by four and indexes into that pointer run, so the
 *           overlay steps from this slot to the helper slot at 0x801F2F80.
 *           Takes no arguments, returns nothing and keeps a 0x18-byte frame
 *           holding only $ra.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching shape: the state-byte advance is the first access group of the exact
 * handler at 0x801F2D3C, which the original emits as one lui/lw pair against
 * the pointer cell followed by lbu/nop/addiu/sb on the record byte; this body
 * reads that cell once, so one local carries it and the cell is spelled through
 * the scratchpad slot macro the target's other lifts use. Calling the stamp
 * routine adds only the 0x18-byte frame with $ra at 0x10.
 */
void stampGlyphGridCellCodesThenAdvanceStateByte1World04Area17013_801F2F48(void) {
  u8* step;

  stampElementsAttachTexture();
  step = SPAD_PTR_SLOT(u8, 0x44u);
  step[1] += 1;
}
