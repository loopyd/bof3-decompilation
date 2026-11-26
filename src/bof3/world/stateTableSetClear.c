#include "bof3/bof3.h"

void clearWorkTableByte3(void);

/* Published scratchpad work-record pointer cell. Entry 0 of this payload's
 * work-byte-1 handler table reads it three times: once for the mode byte that
 * gates the work-table clear, once for the substate-counter store and once for
 * the state-byte advance, so each of those accesses re-materialises the cell.
 * The sibling handler of the same table at 0x801F2CA8 re-reads the cell in the
 * same way, and the target has no map row for the cell, so it keeps the raw
 * address-anchored name.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* @source 0x801F2C48
 * @behavior Run initialiser of this overlay's work-byte-1 state machine: entry 0
 *           of the handler table based at 0x801F4FAC, which this target's exact
 *           dispatcher dispatchStateByte1Table4
 *           indexes with the work record's state byte at +1. That base is a
 *           pointer run inside the payload textbin blob, so this span is reached
 *           by the word at 0x801F4FAC alone and no jal in the payload names it.
 *           The body reads the work record published through the scratchpad
 *           pointer cell 0x1F800044 once for its mode byte at +6; while that
 *           byte reads nonzero it calls the work-table clear helper at
 *           0x801F2E5C, the routine that zeroes the occupancy byte at +3 of the
 *           0x20 records of the 0x2C-byte-stride table at 0x800E4800. It then
 *           re-reads the cell and clears the record's substate counter byte at
 *           +9, and re-reads the cell a third time to advance the state byte at
 *           +1 by one, so the next frame dispatches entry 1 at 0x801F2CA8. Takes
 *           no arguments and returns nothing; the 0x18-byte frame only keeps
 *           $ra at 0x10 across the helper call, whose delay slot stays a nop.
 *           The role of the mode byte at +6 stays anchored to its offset.
 *
 * Name evidence: the role of entry 0 comes from the payload words of the
 * dispatch table and from the target's exact dispatcher; the clear-then-reset-
 * then-advance shape comes from this body's own call target and its own two
 * stores. The cell-reload idiom and the "clear work byte 9, advance work byte 1"
 * naming follow the world04/area175/13 handler at 0x801F3570, whose body differs
 * in the literals it stores and in the cue call only its own arm carries.
 *
 * Each access group reloads the scratchpad pointer cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateTableSetClear(
    void) {
  if (D_1F800044[6] != 0) {
    clearWorkTableByte3();
  }
  D_1F800044[9] = 0;
  D_1F800044[1] += 1;
}
