#include "bof3/bof3.h"

/* Pointer cell at 0x801F1054 in this overlay's trailing data block. It holds
 * the bmagic work record that is being stepped: the frame loop at 0x801EECAC
 * publishes the selected record into this cell with one `sw`, the dispatcher at
 * 0x801EF340 selects a handler with that record's unsigned byte at offset 1, and
 * every handler of this overlay reads the record back through the cell. The cell
 * sits inside a `textbin` row that no map row names, so the reference binds
 * through the target's generated undefined-symbol table. */
extern u8 *D_801F1054; /* @source 0x801F1054 @kind data */

/* @source 0x801F03B0
 * @behavior Clears work-record bytes 0x00 through 0x04 of the record published
 * through the pointer cell at 0x801F1054, storing $zero with one sb per byte.
 * The cell is read once per access, so the original re-materializes the cell
 * address and reloads the record pointer before every store; the last store
 * fills the return delay slot. Both callers run it as the reset arm of a frame
 * countdown: 0x801EF880 (entry 5 of the six-word handler run at 0x801F0404) and
 * 0x801F0220 first decrement byte 0xB of the shared battle work record through
 * 0x801EB4E0, then clear the stepped record here. Takes no arguments, returns
 * nothing and reads or writes no other state. The body has the same shape as
 * clearRecordBytes0Through4BmagicMagic07003_801F006C,
 * clearRecordBytes0Through4BmagicMagic07303_801F0454,
 * clearRecordBytes0Through4BmagicMagic07403_801F0440 and
 * clearRecordBytes0Through4BmagicMagic13103_801F1AC4 and differs from them only
 * in the cell it names.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordBytes0Through4BmagicMagic06303_801F03B0(void) {
  D_801F1054[0] = 0;
  D_801F1054[1] = 0;
  D_801F1054[2] = 0;
  D_801F1054[3] = 0;
  D_801F1054[4] = 0;
}
