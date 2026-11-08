#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D03AC: twenty-eight code pointers,
 * 0x801D03AC through 0x801D0418, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2 and 18. The Splat
 * `textbin` row T_801D03AC places the words, so the run is only referenced here.
 * This dispatcher indexes it from 0x801D0404, so its base is entry 22 of that run
 * and the six entries it reaches are 0x801CE404, 0x801CE620, 0x801CF6C8,
 * 0x801CE610, 0x801CE84C and 0x801CF8FC. The neighbouring base 0x801D0410,
 * indexed by the dispatcher at 0x801D0368, holds the last three of those. */
extern void (*D_801D0404[])(void); /* @source 0x801D0404 @kind data */

/* @source 0x801D0324
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801D0404 with no arguments,
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
void dispatchWorkHalfword2cTable0404PlcharPlp46700_801D0324(void) {
  D_801D0404[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
