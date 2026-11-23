#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF900: twenty-one code pointers, the words
 * 0x801CF900 through 0x801CF950, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 5 and 11. The leading
 * sixteen words sit inside the 0x801CF900 `textbin` row and the trailing five in
 * the 0x801CF940 row, which the reviewed Splat layout places, so the run is only
 * referenced here. This dispatcher indexes the run from 0x801CF93C, the last
 * word of the `textbin` row, so its base is entry 15 of that run and the six
 * entries it can reach are 0x801CE404, 0x801CE620, 0x801CEC1C, 0x801CE610,
 * 0x801CE82C and 0x801CEE50. The neighbouring base 0x801CF948 used by the
 * sibling dispatcher at 0x801CF8BC sits three entries further on. */
extern void (*D_801CF93C[])(void); /* @source 0x801CF93C @kind data */

/* @source 0x801CF878
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CF93C with no arguments,
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
void callWorkTablef93Plchar(void) {
  D_801CF93C[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
