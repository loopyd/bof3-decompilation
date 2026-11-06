#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0A54: thirty code pointers, entry 0 =
 * 0x801CE448 through entry 29 = 0x801CFB50, holding this payload's functions plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 8. The target's
 * Splat `textbin` row of the same base places the words, so the run is only
 * referenced here. This dispatcher indexes it from 0x801D0AB4, so its base is
 * entry 24 and the reachable entries are 0x801CE404 through 0x801CFB50. */
extern void (*D_801D0AB4[])(void); /* @source 0x801D0AB4 @kind data */

/* @source 0x801D09CC
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801D0AB4 with no arguments,
 *           discarding any result. The original does not bound the index before
 *           the scaled table load. It reads no other state and writes none; the
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the indirect call, and the slots after the
 *           index halfword and after the table load stay nops. Neither this
 *           function's address nor the table base appears in any word of this
 *           payload, so it is an overlay entry reached from outside the payload;
 *           its prologue and return path alone place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkHalfword2cTable0ab4PlcharPlp02600_801D09CC(void) {
  D_801D0AB4[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
