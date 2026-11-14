#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFFE8: seven code pointers, entry 0 =
 * 0x801CFE4C through entry 6 = 0x801CFB30, all pointing at functions of this
 * payload. That run currently sits inside the target's `textbin T_801CFFE8`
 * boundary, so it is only referenced here. This dispatcher indexes it from
 * 0x801CFFF8, so its base is entry 4 and the entries this payload places from
 * that base are 0x801CE72C, 0x801CF338 and 0x801CFB30. The sibling dispatcher at
 * 0x801CFF20 indexes the same bytes from entry 1 (0x801CFFEC), and the two bodies
 * differ in that table base alone. */
extern void (*D_801CFFF8[])(void); /* @source 0x801CFFF8 @kind data */

/* @source 0x801CFF64
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFFF8 with no arguments,
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
void dispatchWorkHalfword2cTablefff8PlcharPlp02500_801CFF64(void) {
  D_801CFFF8[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
