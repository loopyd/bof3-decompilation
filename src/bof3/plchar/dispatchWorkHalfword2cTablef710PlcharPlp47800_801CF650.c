#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: twenty code pointers, 0x801CF6D8 through
 * 0x801CF724, holding this payload's functions plus the shared main-executable
 * handler 0x801B2D54 at entries 2, 5 and 11. The Splat `textbin` row T_801CF6D8
 * places the words, so the run is only referenced here. This dispatcher indexes
 * it from 0x801CF710, so its base is entry 14 and the entries the run still holds
 * from there are 0x801CE404, 0x801CE620, 0x801CF27C, 0x801CE610, 0x801CE854 and
 * 0x801CF488. */
extern void (*D_801CF710[])(void); /* @source 0x801CF710 @kind data */

/* @source 0x801CF650
 * @behavior Overlay step dispatch on the work record halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CF710 with no arguments,
 *           discarding any result. The original does not bound the index before
 *           the scaled table load. It reads no other state and writes none; the
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the indirect call, and the slots after the
 *           index halfword and after the table load stay nops. Neither this
 *           function's address nor the table base appears in any word of this
 *           payload and no jal targets it, so it is an overlay entry reached
 *           from outside the payload; its prologue and return path alone place
 *           the boundary, which ends at 0x801CF694 where func_801CF694 begins.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkHalfword2cTablef710PlcharPlp47800_801CF650(void) {
  D_801CF710[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
