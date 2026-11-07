#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFBC4: seven code pointers, entry 0 =
 * 0x801CFA28 through entry 6 = 0x801CF70C, holding this payload's functions. That
 * row currently sits inside the `asm func_801CFBC4` boundary, so the run is only
 * referenced here. This dispatcher indexes it from 0x801CFBD4, entry 4 of that run,
 * so the three entries it can reach are 0x801CE72C, 0x801CF338 and 0x801CF70C. The
 * neighbouring base 0x801CFBC8 used by the sibling dispatcher at 0x801CFAFC holds
 * entries 1 to 3 of the same run. */
extern void (*D_801CFBD4[])(void); /* @source 0x801CFBD4 @kind data */

/* @source 0x801CFB40
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFBD4 with no arguments,
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
void dispatchWorkHalfword2cTablefbd4PlcharPlp01500_801CFB40(void) {
  D_801CFBD4[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
