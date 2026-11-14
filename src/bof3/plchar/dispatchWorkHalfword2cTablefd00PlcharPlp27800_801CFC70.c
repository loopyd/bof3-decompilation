#include "bof3/emi/dispatchers.h"

/* This overlay's trailing handler run at 0x801CFCF4: the payload's last six code
 * pointers, 0x801CFCF4 through 0x801CFD08, holding this payload's own functions
 * 0x801CE404, 0x801CEBFC, 0x801CF858, 0x801CE610, 0x801CEE30 and 0x801CFA64. The
 * wider run at 0x801CFCB4 ends at 0x801CFCF0, so these six words are their own run
 * and the target's `textbin T_801CFCF4` row places them; the run is only
 * referenced here. This dispatcher indexes it from 0x801CFD00, so its base is
 * entry 3 of that run, and the three entries it reaches are 0x801CE610,
 * 0x801CEE30 and 0x801CFA64. The neighbouring 0x801CFCF4, entry 0, is the base
 * the sibling dispatcher at 0x801CFC2C uses. Because this base sits three words
 * from the end of the run, an index above two reads past the end of the payload. */
extern void (*D_801CFD00[])(void); /* @source 0x801CFD00 @kind data */

/* @source 0x801CFC70
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFD00 with no arguments,
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
void dispatchWorkHalfword2cTablefd00PlcharPlp27800_801CFC70(void) {
  D_801CFD00[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
