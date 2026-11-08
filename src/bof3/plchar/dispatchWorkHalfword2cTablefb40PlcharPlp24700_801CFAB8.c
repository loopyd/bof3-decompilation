#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAFC: twenty code pointers filling the
 * payload's trailing read-only block, holding this payload's own functions plus
 * the shared main-executable handler 0x801B2D54 at run words 2, 7 and 10. The
 * target's Splat `textbin` row places the whole run, so it is only referenced
 * here. This dispatcher indexes it from 0x801CFB40, run word 17, so the three
 * words still inside the run are 0x801CE610, 0x801CEE08 and 0x801CF04C. The
 * overlay's two byte-2 dispatchers base their own calls at run words 8
 * (0x801CFB1C) and 11 (0x801CFB28), six and nine words earlier. */
extern void (*D_801CFB40[])(void); /* @source 0x801CFB40 @kind data */

/* @source 0x801CFAB8
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFB40 with no arguments,
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
void dispatchWorkHalfword2cTablefb40PlcharPlp24700_801CFAB8(void) {
  D_801CFB40[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
