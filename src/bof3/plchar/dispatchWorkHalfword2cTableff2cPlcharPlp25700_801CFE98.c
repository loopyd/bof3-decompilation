#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFEDC: twenty-three code pointers spanning
 * the two `textbin` rows that follow this dispatcher, `func_801CFEDC`
 * (0x801CFEDC through 0x801CFF18) and `T_801CFF1C` (0x801CFF1C through
 * 0x801CFF34), holding entry 0 = 0x801CE448 through entry 22 = 0x801CF42C: this
 * payload's own functions plus the shared main-executable handler 0x801B2D54 at
 * entries 2, 7 and 13. Those rows are only referenced here. This dispatcher
 * indexes the run from 0x801CFF2C, so its base is entry 20 and the three entries
 * it reaches are 0x801CE610, 0x801CEE08 and 0x801CF42C. The neighbouring base
 * 0x801CFF20 used by the sibling dispatcher at 0x801CFE54 is entry 17 of the
 * same run. */
extern void (*D_801CFF2C[])(void); /* @source 0x801CFF2C @kind data */

/* @source 0x801CFE98
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFF2C with no arguments,
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
void dispatchWorkHalfword2cTableff2cPlcharPlp25700_801CFE98(void) {
  D_801CFF2C[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
