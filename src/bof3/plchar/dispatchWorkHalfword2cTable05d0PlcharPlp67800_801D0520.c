#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0564: thirty code pointers, 0x801D0564
 * through 0x801D05D8, holding this payload's functions plus the shared
 * main-executable handler 0x801B2D54 at entries 15 and 21. The Splat `textbin`
 * row T_801D0564 places the words, so the run is only referenced here. This
 * dispatcher indexes it from 0x801D05D0, so its base is entry 27 and the three
 * entries it can reach are 0x801CE630, 0x801CF6E0 and 0x801D0314. The
 * neighbouring base 0x801D05C4 used by the sibling dispatcher at 0x801D04DC
 * reaches entries 24 to 29 of the same run. */
extern void (*D_801D05D0[])(void); /* @source 0x801D05D0 @kind data */

/* @source 0x801D0520
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801D05D0 with no arguments,
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
void dispatchWorkHalfword2cTable05d0PlcharPlp67800_801D0520(void) {
  D_801D05D0[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
