#include "bof3/bof3.h"

/* Pointer cell at 0x801F17B4 in this overlay's trailing data block. It holds
 * the bmagic work record being stepped: func_801EF33C selects a handler with
 * that record's byte 1 through the table at 0x801F01B0, and func_801EF380
 * selects one with byte 2. The cell is reloaded once per access, so the
 * original re-materializes its address (lui $v0,0x801F / lw
 * $v0,%lo(D_801F17B4)($v0)) before each byte store. The cell sits inside the
 * reviewed trailing data row, which no map row names, so the reference is bound
 * by the target's generated undefined-symbol table. */
extern u8 *D_801F17B4; /* @source 0x801F17B4 @kind data */

/* @source 0x801F006C
 * @behavior Clears work-record bytes 0x00 through 0x04 of the record published
 * through the pointer cell at 0x801F17B4, storing $zero with one sb per byte.
 * The cell is read once per access, so the original re-materializes the cell
 * address and reloads the record pointer before every store; the last sb sits
 * in the return delay slot. Takes no arguments, returns nothing and reads or
 * writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordBytes0Through4BmagicMagic07103_801F006C(void) {
  D_801F17B4[0] = 0;
  D_801F17B4[1] = 0;
  D_801F17B4[2] = 0;
  D_801F17B4[3] = 0;
  D_801F17B4[4] = 0;
}
