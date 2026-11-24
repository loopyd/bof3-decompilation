#include "bof3/bof3.h"

/* Pointer cell at 0x801F17B4 in this overlay's trailing data block. It holds
 * the bmagic work record being stepped: the frame loop at 0x801EEC28 publishes
 * the selected entry of the 0x80-entry, 0x2C-byte record array based at
 * 0x801F01B4 into this cell with one `sw` at 0x801EECE8, and
 * callWorkByte1Handler3 selects a handler
 * with that record's unsigned byte at offset 1 while func_801EF380 selects one
 * with byte 2. The cell is reloaded once per access, so the original
 * re-materializes its address (lui $v0,0x801F / lw $v0,%lo(D_801F17B4)($v0))
 * before each byte store. The cell sits inside the trailing `textbin` row
 * labelled 0x801F00BC, which no map row names, so the reference binds through
 * the target's generated undefined-symbol table. */
extern u8 *D_801F17B4; /* @source 0x801F17B4 @kind data */

/* @source 0x801F006C
 * @behavior Clears work-record bytes 0x00 through 0x04 of the record published
 * through the pointer cell at 0x801F17B4, storing $zero with one sb per byte.
 * The cell is read once per access, so the original re-materializes the cell
 * address and reloads the record pointer before every store; the last sb sits
 * in the return delay slot. The only reference to this body in the payload is
 * the jal at 0x801EF7F8 inside func_801EF72C, which reaches it once the
 * record's byte 5 has been decremented to zero, so the cleared byte 0 retires
 * the record from the frame loop's bit-0 test and the cleared byte 1 zeroes the
 * dispatcher index. It takes no arguments, returns nothing and reads or writes
 * no other state; the 0x50-byte span ends exactly at the trailing data row. The
 * 80 bytes are byte-identical to the sibling lifts at emi/bmagic/magic070/03
 * and emi/bmagic/magic071/03 0x801F006C, proven by cmp of the extracted spans
 * and by the clone dry run reporting byte_identical true with no rewrites.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateClearRecordBytes3(void) {
  D_801F17B4[0] = 0;
  D_801F17B4[1] = 0;
  D_801F17B4[2] = 0;
  D_801F17B4[3] = 0;
  D_801F17B4[4] = 0;
}
