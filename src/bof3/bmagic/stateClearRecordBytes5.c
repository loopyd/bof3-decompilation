#include "bof3/bof3.h"

/* Pointer cell that holds the bmagic work record being stepped. It is also
 * read by the dispatcher at 0x801EF710 in this overlay, which selects a
 * handler with the record's byte 1. The cell is reloaded once per access, so
 * the original re-materializes its address (lui $v0,0x801F /
 * lw $v0,%lo(stateWorkRecordPointer)($v0)) before every byte store. The reviewed trailing
 * data row of this target does not name the cell, so the reference resolves
 * through the target's generated undefined-symbol table. */
extern u8 *stateWorkRecordPointer; /* @source 0x801F20DC @kind data */

/* @source 0x801F0440
 * @behavior Clears work-record bytes 0x00 through 0x04 of the record published
 * through the pointer cell at 0x801F20DC, storing $zero with one sb per byte.
 * The cell is read once per access, so the original re-materializes the cell
 * address and reloads the record pointer before every store; the last sb sits
 * in the return delay slot. Takes no arguments, returns nothing and reads or
 * writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateClearRecordBytes5(void) {
  stateWorkRecordPointer[0] = 0;
  stateWorkRecordPointer[1] = 0;
  stateWorkRecordPointer[2] = 0;
  stateWorkRecordPointer[3] = 0;
  stateWorkRecordPointer[4] = 0;
}
