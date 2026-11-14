#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAB8: twenty-three code pointers,
 * 0x801CFAB8 through 0x801CFB10, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 8 and 14. The run ends
 * with the payload at 0x801CFB14 and the target's `textbin T_801CFAB8` row
 * places its words, so it is only referenced here. This dispatcher indexes it
 * from 0x801CFAFC, so its base is entry 17 of that run and the six entries it
 * reaches are 0x801CE404, 0x801CEA00, 0x801CF65C, 0x801CE610, 0x801CEC34 and
 * 0x801CF868. The neighbouring base 0x801CFB08, indexed by the dispatcher at
 * 0x801CFA74, holds the last three of those. */
extern void (*D_801CFAFC[])(void); /* @source 0x801CFAFC @kind data */

/* @source 0x801CFA30
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFAFC with no arguments,
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
void dispatchWorkHalfword2cTablefafcPlcharPlp57800_801CFA30(void) {
  D_801CFAFC[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
