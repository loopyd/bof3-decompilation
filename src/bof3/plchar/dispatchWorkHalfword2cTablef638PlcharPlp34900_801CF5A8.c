#include "bof3/emi/dispatchers.h"

/* The dispatcher table this body indexes is the six-word code-pointer run at
 * 0x801CF62C: entry 0 = 0x801CE404 through entry 5 = 0x801CF554. This target's
 * Splat row at 0x801CF62C still declares that run as the function func_801CF62C
 * although every one of its six words is a code pointer, so the run is only
 * referenced here. This dispatcher indexes it from 0x801CF638, word 3 of the
 * six, so its base is that interior word and the three entries it reaches are
 * 0x801CE610, 0x801CF534 and 0x801CF554. The sixteen-word pointer run at
 * 0x801CF5EC, which the byte-slot dispatchers of this overlay index, ends three
 * words below this run and is a separate table. */
extern void (*D_801CF638[])(void); /* @source 0x801CF638 @kind data */

/* @source 0x801CF5A8
 * @behavior Overlay step dispatch on the work record halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CF638 with no arguments,
 *           discarding any result. The original does not bound the index before
 *           the scaled table load. It reads no other state and writes none; the
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the indirect call, and the slots after the
 *           index halfword and after the table load stay nops. Neither this
 *           function's address nor the table base appears in any word of this
 *           payload and no jal targets it, so it is an overlay entry reached
 *           from outside the payload; its prologue and return path alone place
 *           the boundary, which ends exactly where the 0x801CF5EC pointer run
 *           begins.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkHalfword2cTablef638PlcharPlp34900_801CF5A8(void) {
  D_801CF638[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
