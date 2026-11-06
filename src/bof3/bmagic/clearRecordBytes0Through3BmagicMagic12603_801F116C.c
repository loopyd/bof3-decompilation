#include "bof3/bof3.h"

/* Pointer cell at 0x801F22BC in this overlay's trailing data block. It holds
 * the bmagic record that is being stepped: func_801F0960 indexes a handler
 * table by that record's unsigned byte at offset 2, func_801F0DA4 steps its
 * bytes at offsets 4, 6 and 7 and its words at offsets 8, 0x14 and 0x18, and
 * func_801F0B34 reads the same cell. The cell is read once per access, so the
 * original re-materializes its address (lui $v0,0x801F / lw
 * $v0,%lo(D_801F22BC)($v0)) before each byte store. The cell sits inside the
 * reviewed trailing data row, which no map row names, so the reference is bound
 * by the target's generated undefined-symbol table. */
extern u8 *D_801F22BC; /* @source 0x801F22BC @kind data */

/* @source 0x801F116C
 * @behavior Clears record bytes 0x00 through 0x03 of the record published
 * through the pointer cell at 0x801F22BC, storing $zero with one sb per byte.
 * The cell is read once per access, so the original re-materializes the cell
 * address and reloads the record pointer before every store. Takes no
 * arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordBytes0Through3BmagicMagic12603_801F116C(void) {
  D_801F22BC[0] = 0;
  D_801F22BC[1] = 0;
  D_801F22BC[2] = 0;
  D_801F22BC[3] = 0;
}
