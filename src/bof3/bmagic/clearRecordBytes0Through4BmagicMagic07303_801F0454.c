#include "bof3/bof3.h"

/* Pointer cell at 0x801F21F0 in this overlay's trailing data block. It holds
 * the bmagic work record that is being stepped: func_801EEC28 publishes the
 * record there with one store, the dispatcher at 0x801EF724 selects a handler
 * with that record's byte 1, and func_801EFBF8, func_801EFE2C and func_801F00C4
 * read it. The cell is reloaded once per access, so the original re-materializes
 * its address (lui $v0,0x801F / lw $v0,%lo(D_801F21F0)($v0)) before each byte
 * store. The cell sits inside the reviewed trailing data row, which no map row
 * names, so the reference is bound by the target's generated undefined-symbol
 * table. */
extern u8 *D_801F21F0; /* @source 0x801F21F0 @kind data */

/* @source 0x801F0454
 * @behavior Clears work-record bytes 0x00 through 0x04 of the record published
 * through the pointer cell at 0x801F21F0, storing $zero with one sb per byte.
 * The cell is read once per access, so the original re-materializes the cell
 * address and reloads the record pointer before every store. Takes no
 * arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordBytes0Through4BmagicMagic07303_801F0454(void) {
  D_801F21F0[0] = 0;
  D_801F21F0[1] = 0;
  D_801F21F0[2] = 0;
  D_801F21F0[3] = 0;
  D_801F21F0[4] = 0;
}
