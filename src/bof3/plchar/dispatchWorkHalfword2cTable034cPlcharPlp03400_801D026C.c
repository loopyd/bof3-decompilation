#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D02F4: twenty-eight code pointers, entry 0
 * = 0x801CE448 through entry 27 = 0x801D025C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 8 and 21.
 * The Splat `textbin` row T_801D02F4 places the words, so the run is only
 * referenced here. This dispatcher indexes it from 0x801D034C, so its base is
 * entry 22 and its reachable entries are the six words 0x801CE404, 0x801CF12C,
 * 0x801D0050, 0x801CE72C, 0x801CF338 and 0x801D025C through the end of the run.
 * Its own address is not one of those words, so no code in this payload enters
 * it through this table. */
extern void (*D_801D034C[])(void); /* @source 0x801D034C @kind data */

/* @source 0x801D026C
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801D034C with no arguments,
 *           discarding any result. The original does not bound the index before
 *           the scaled table load. It reads no other state and writes none; the
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the indirect call, and the slots after the
 *           index halfword and after the table load stay nops. Neither this
 *           function's address nor the table base appears in any word of this
 *           payload and no jal reaches it, so it is an overlay entry reached
 *           from outside the payload; its prologue and return path alone place
 *           the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkHalfword2cTable034cPlcharPlp03400_801D026C(void) {
  D_801D034C[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
