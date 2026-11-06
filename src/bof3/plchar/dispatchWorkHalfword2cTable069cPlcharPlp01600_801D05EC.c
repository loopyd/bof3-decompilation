#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0630: thirty code pointers, entry 0 =
 * 0x801CE448 through entry 29 = 0x801CF72C, holding this payload's functions plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 8. The Splat
 * `textbin` row of the same name places the words, so the run is only referenced
 * here. This dispatcher indexes it from 0x801D069C, so its base is entry 27
 * (0x801CE72C) and the entries it can reach inside the run are entries 27 through
 * 29: 0x801CE72C, 0x801CF338 and 0x801CF72C. The neighbouring base 0x801D0690 used
 * by the sibling dispatcher at 0x801D05A8 is entry 24 of the same run. */
extern void (*D_801D069C[])(void); /* @source 0x801D069C @kind data */

/* @source 0x801D05EC
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801D069C with no arguments,
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
void dispatchWorkHalfword2cTable069cPlcharPlp01600_801D05EC(void) {
  D_801D069C[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
