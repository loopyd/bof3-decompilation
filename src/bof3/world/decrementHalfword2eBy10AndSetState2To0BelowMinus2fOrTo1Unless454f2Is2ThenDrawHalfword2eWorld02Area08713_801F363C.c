#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* scratchpadWorkRecord;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;

/* The local drawer narrows both arguments with `sll`/`sra 16` in its own
 * prologue and neither call site consumes a result. */
void drawMarkerSpriteLayersSelectedByMapCodeThenAppendPanelWorld02Area08713_801F3B34(
    s16 arg0, s16 arg1);

/* @source 0x801F363C
 * @behavior Entry 3 of the byte-2 handler table based at 0x801F5750 in this
 *           overlay, published by the pointer word at 0x801F575C, for the work
 *           record read from the scratchpad pointer cell 0x1F800044. Subtracts
 *           0x10 from the signed halfword at record offset 0x2E and stores the
 *           truncated halfword. While the stored signed value stays below
 *           -0x2F it writes 0 to state byte 0x02; unless the shared mode byte
 *           at 0x801454F2 is 2 it then writes 1 to state byte 0x02 through a
 *           second read of the pointer cell. Finally it reads that cell once
 *           more and calls the local drawer at 0x801F3B34 with 0x10 and the
 *           signed halfword at record offset 0x2E, discarding any result. Takes
 *           no arguments, uses no result and saves only the return address in
 *           its 0x18-byte frame. Its 0x84 original bytes were byte-identical to
 *           the then exact lift
 *           decrementHalfword2eBy10AndSetState2To0BelowMinus2fOrTo1Unless454f2Is2ThenDrawHalfword2eWorld02Area08813_801F416C
 *           apart from the single `jal` immediate word, so this source follows
 *           that sibling's statement order and local shapes with this target's
 *           own drawer address and bindings.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword2eBy10AndSetState2To0BelowMinus2fOrTo1Unless454f2Is2ThenDrawHalfword2eWorld02Area08713_801F363C(
    void) {
  u8* record;
  u8* draw;
  s16* half;

  record = scratchpadWorkRecord;
  half = (s16*)(record + 0x2E);
  *half = (s16)(*half - 0x10);
  if (*half < -0x2F) {
    record[2] = 0;
  }
  if (D_801454F2 != 2) {
    scratchpadWorkRecord[2] = 1;
  }
  draw = scratchpadWorkRecord;
  drawMarkerSpriteLayersSelectedByMapCodeThenAppendPanelWorld02Area08713_801F3B34(
      0x10, FIELD_REF(s16, draw, 0x2E));
}
