#include "bof3/bof3.h"

/* Pointer cell at 0x801F17B0 in this overlay's trailing data block. It holds
 * the bmagic work record that is being stepped: the frame loop at 0x801EEC28
 * publishes 0x801F01B0 + i * 0x2C there with one store (`sw $v0,
 * %lo(D_801F17B0)($at)` at 0x801EECEC), the handler at 0x801EF728 steps that
 * record, and the dispatcher at 0x801EF338 indexes the handler table at
 * 0x801F01AC by the record's unsigned byte at offset 1. The cell is reloaded
 * once per access, so the original re-materializes its address (lui $v0,0x801F
 * / lw $v0,%lo(D_801F17B0)($v0)) before each byte store. The cell sits inside
 * the trailing data block, which no map row names, so the reference binds
 * through the target's generated undefined-symbol table. */
extern u8 *D_801F17B0; /* @source 0x801F17B0 @kind data */

/* @source 0x801F0068
 * @behavior Clears work-record bytes 0x00 through 0x04 of the record published
 * through the pointer cell at 0x801F17B0, storing $zero with one sb per byte.
 * The cell is read once per access, so the original re-materializes the cell
 * address and reloads the record pointer before every store; the last sb sits
 * in the return delay slot. Its only caller is the record handler at
 * 0x801EF728, which reaches it after decrementing the record's byte at offset
 * 0x05 to zero. Takes no arguments, returns nothing and reads or writes no
 * other state. The 80-byte body is identical to emi/bmagic/magic073/03
 * 0x801F0454 and emi/bmagic/magic074/03 0x801F0440 up to the five reload
 * immediates of the cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordBytes0Through4BmagicMagic16203_801F0068(void) {
  D_801F17B0[0] = 0;
  D_801F17B0[1] = 0;
  D_801F17B0[2] = 0;
  D_801F17B0[3] = 0;
  D_801F17B0[4] = 0;
}
