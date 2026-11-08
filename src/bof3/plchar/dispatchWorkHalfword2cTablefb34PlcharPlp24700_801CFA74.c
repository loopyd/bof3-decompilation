#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAFC: twenty code pointers filling the
 * payload's trailing read-only block, holding this payload's functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 7 and 10. The target's
 * Splat `textbin` row places the whole run, so it is only referenced here. This
 * dispatcher indexes it from 0x801CFB34, its fifteenth entry, so the six
 * reachable entries are 0x801CE404, 0x801CEBFC, 0x801CEE18, 0x801CE610,
 * 0x801CEE08 and 0x801CF04C. Those bases are reached three and eight entries
 * further in by the overlay's two byte-2 dispatchers at 0x801CEE18 (base
 * 0x801CFB1C) and 0x801CF04C (base 0x801CFB28). */
extern void (*D_801CFB34[])(void); /* @source 0x801CFB34 @kind data */

/* @source 0x801CFA74
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFB34 with no arguments,
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
void dispatchWorkHalfword2cTablefb34PlcharPlp24700_801CFA74(void) {
  D_801CFB34[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
