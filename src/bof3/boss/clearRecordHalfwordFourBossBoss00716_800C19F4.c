#include "bof3/bof3.h"

/* @source 0x800C19F4
 * @behavior Clears the halfword at +0x04 of the record reached through the
 * pointer cell at 0x80146390 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: `lui`/`lw` read the cell at 0x80146390 and `sh $zero, 4($v0)`
 * lands in the `jr $ra` delay slot. The pointee layout is unrecovered, so the
 * fixed pointer cell stays in the memory API.
 */
void clearRecordHalfwordFourBossBoss00716_800C19F4(void) {
  PSX_REF(u16 *, 0x80146390u)[2] = 0;
}
