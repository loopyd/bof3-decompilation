#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFEDC: twenty-three code pointers, entry 0
 * = 0x801CE448 through entry 22 = 0x801CF42C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 7 and 13. The
 * run's tail sits inside the following `textbin T_801CFF1C` row, so the run is
 * only referenced here. This dispatcher indexes it from 0x801CFF20, so its base
 * is entry 17 of that run and the six entries it reaches are 0x801CE404,
 * 0x801CEBFC, 0x801CF1F8, 0x801CE610, 0x801CEE08 and 0x801CF42C. The
 * neighbouring base 0x801CFF2C used by the sibling dispatcher at 0x801CFE98
 * holds the last three of those. */
extern void (*D_801CFF20[])(void); /* @source 0x801CFF20 @kind data */

/* @source 0x801CFE54
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFF20 with no arguments,
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
void dispatchWorkHalfword2cTableff20PlcharPlp25700_801CFE54(void) {
  D_801CFF20[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
