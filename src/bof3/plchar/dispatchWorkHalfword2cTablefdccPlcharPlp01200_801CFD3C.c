#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFDC0: six code pointers, entry 0 =
 * 0x801CE404 through entry 5 = 0x801CF70C, all pointing at functions of this
 * payload. That run sits inside the target's `textbin T_801CFDC0` boundary, so it
 * is only referenced here. This dispatcher indexes it from 0x801CFDCC, so its base
 * is entry 3 and the entries this payload places from that base are 0x801CE72C,
 * 0x801CF338 and 0x801CF70C. The sibling dispatcher at 0x801CFCF8 indexes the same
 * bytes from entry 0, and the two bodies differ in that table base alone. */
extern void (*D_801CFDCC[])(void); /* @source 0x801CFDCC @kind data */

/* @source 0x801CFD3C
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFDCC with no arguments,
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
void dispatchWorkHalfword2cTablefdccPlcharPlp01200_801CFD3C(void) {
  D_801CFDCC[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
