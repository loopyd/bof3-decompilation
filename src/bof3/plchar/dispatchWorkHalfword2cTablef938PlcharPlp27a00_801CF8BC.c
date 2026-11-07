#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF900: seventeen code pointers, entry 0 =
 * 0x801CE448 through entry 16 = 0x801CF868, holding this payload's functions and
 * three entries of a companion overlay at 0x801B2D54. That run currently sits
 * inside the `textbin T_801CF900` boundary, so it is only referenced here. This
 * dispatcher indexes it from 0x801CF938, so its base is entry 14 of that run, and
 * the three entries it reaches are 0x801CE610, 0x801CEE30 and 0x801CF868. The
 * neighbouring base 0x801CF92C used by the sibling dispatcher at 0x801CF878 holds
 * the three entries before it. */
extern void (*D_801CF938[])(void); /* @source 0x801CF938 @kind data */

/* @source 0x801CF8BC
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CF938 with no arguments,
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
void dispatchWorkHalfword2cTablef938PlcharPlp27a00_801CF8BC(void) {
  D_801CF938[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
