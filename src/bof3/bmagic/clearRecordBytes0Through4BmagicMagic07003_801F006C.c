#include "bof3/bof3.h"

/* Pointer cell at 0x801F17B4 in this overlay's trailing data block. It holds
 * the bmagic work record that is being stepped: the frame loop at 0x801EEC28
 * publishes the selected entry of the 0x80-entry, 0x2C-byte record array based
 * at 0x801F01B4 into this cell with one `sw` at 0x801EECE8, and the dispatcher
 * at 0x801EF33C selects a handler with that record's unsigned byte at offset 1.
 * Every handler of this overlay reads the record back through the cell, so the
 * original re-materializes the cell address (lui $v0,0x801F /
 * lw $v0,%lo(D_801F17B4)($v0)) before each access. The cell sits inside the
 * reviewed trailing `textbin` row labelled 0x801F00BC, which no map row names,
 * so the reference binds through the target's generated undefined-symbol
 * table. */
extern u8 *D_801F17B4; /* @source 0x801F17B4 @kind data */

/* @source 0x801F006C
 * @behavior Clears work-record bytes 0x00 through 0x04 of the record published
 * through the pointer cell at 0x801F17B4, storing $zero with one sb per byte.
 * The cell is read once per access, so the original re-materializes the cell
 * address and reloads the record pointer before every store; the last store
 * fills the return delay slot. Takes no arguments, returns nothing and reads or
 * writes no other state. The body has the same shape as
 * clearRecordBytes0Through4BmagicMagic07303_801F0454,
 * clearRecordBytes0Through4BmagicMagic07403_801F0440 and
 * clearRecordBytes0Through4BmagicMagic13103_801F1AC4 and differs from them only
 * in the cell it names; the 0x50-byte span is also byte-identical to the
 * still-unlifted emi/bmagic/magic072/03 0x801F006C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordBytes0Through4BmagicMagic07003_801F006C(void) {
  D_801F17B4[0] = 0;
  D_801F17B4[1] = 0;
  D_801F17B4[2] = 0;
  D_801F17B4[3] = 0;
  D_801F17B4[4] = 0;
}
