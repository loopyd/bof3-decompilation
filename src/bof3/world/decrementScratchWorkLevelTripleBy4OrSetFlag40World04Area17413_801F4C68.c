#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-reads the cell through
 * its own address load for every store group after the first, so only the
 * opening halfword/level accesses share one load, as a local pointer.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Pointer cell of the shared record whose halfword at 0x0A this step consumes.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* @source 0x801F4C68
 * @behavior Per-frame step of the scratch work record published through the
 * pointer cell at 0x1F800044: adds 8 to the record halfword at 0x3E, and while
 * the signed level byte 0x5D has not yet reached -0x80 subtracts 4 from that
 * byte and from the two following level bytes 0x5E and 0x5F - each of the two
 * through its own fresh load of the pointer cell - and then consumes two units
 * of the halfword at 0x0A of the record published through the global pointer
 * cell 0x80148210. Once byte 0x5D is -0x80 it instead sets flag bit 0x40 of
 * record byte 0 and stores 0 into work byte 4, and that arm leaves the
 * 0x80148210 halfword untouched. Work byte 4 is the selector the neighbouring
 * dispatchWorkByte4LocalHandlersWorld04Area17413_801F4AB4 indexes its two-entry
 * handler table with, and the table's first entry func_801F4B0C stores 1 back
 * into byte 4 while arming the same three level bytes, so this step returns the
 * record to that arming handler once the levels reach -0x80. Bytes 0x3E, 0x5D,
 * 0x5E, 0x5F and 0 are read back only by these handlers, so their roles stay
 * anchored to the original offsets. Takes no arguments, calls nothing and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementScratchWorkLevelTripleBy4OrSetFlag40World04Area17413_801F4C68(
    void) {
  u8* record;

  record = D_1F800044;
  *(u16*)(record + 0x3e) += 8;
  if (*(s8*)(record + 0x5d) != -0x80) {
    *(s8*)(record + 0x5d) = *(s8*)(record + 0x5d) - 4;
    D_1F800044[0x5e] -= 4;
    D_1F800044[0x5f] -= 4;
    D_80148210[5] -= 2;
  } else {
    record[0] |= 0x40;
    D_1F800044[4] = 0;
  }
}
