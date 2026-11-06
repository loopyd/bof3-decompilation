#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF900: seventeen code pointers, entry 0 =
 * 0x801CE448 through entry 16 = 0x801CF868, holding this payload's functions and
 * two entries of a companion overlay at 0x801B2D54. That run currently sits
 * inside the `textbin T_801CF900` boundary, so it is only referenced here. This
 * dispatcher indexes it from 0x801CF92C, so its base is entry 11 of that run, and
 * the three entries it reaches are 0x801CE404, 0x801CEBFC and 0x801CF858. The
 * neighbouring base 0x801CF938 used by the sibling dispatcher at 0x801CF8BC holds
 * the next three entries. */
extern void (*D_801CF92C[])(void); /* @source 0x801CF92C @kind data */

/* @source 0x801CF878
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CF92C with no arguments,
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
void dispatchWorkHalfword2cTablef92cPlcharPlp27a00_801CF878(void) {
  D_801CF92C[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
