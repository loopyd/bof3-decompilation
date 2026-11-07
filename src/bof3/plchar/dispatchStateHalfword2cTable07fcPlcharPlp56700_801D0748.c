#include "bof3/emi/dispatchers.h"

/* This overlay's trailing handler run at 0x801D07EC: seven code pointers, entry 0
 * (0x801D07EC) = 0x801D0208, entry 1 (0x801D07F0) = 0x801CE404, entry 2
 * (0x801D07F4) = 0x801CEA00, entry 3 (0x801D07F8) = 0x801CFAA8, entry 4
 * (0x801D07FC) = 0x801CE610, entry 5 (0x801D0800) = 0x801CEC2C, entry 6
 * (0x801D0804) = 0x801CFCDC, holding this payload's functions. That row currently
 * sits inside the `asm func_801D07EC` boundary, so the run is only referenced here.
 * This dispatcher indexes it from 0x801D07FC, so its base is entry 4 and the three
 * entries it reaches are 0x801CE610, 0x801CEC2C and 0x801CFCDC. The neighbouring
 * base 0x801D07F0 used by the sibling dispatcher at 0x801D0704 holds the three
 * entries before it. */
extern void (*D_801D07FC[])(void); /* @source 0x801D07FC @kind data */

/* @source 0x801D0748
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801D07FC with no arguments,
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
void dispatchStateHalfword2cTable07fcPlcharPlp56700_801D0748(void) {
  D_801D07FC[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
