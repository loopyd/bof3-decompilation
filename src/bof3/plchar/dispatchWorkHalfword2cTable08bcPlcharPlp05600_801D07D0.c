#include "bof3/emi/dispatchers.h"

/* This overlay's trailing handler run at 0x801D08B8: seven code pointers, entry
 * 0 (0x801D08B8) = 0x801D042C, entry 1 (0x801D08BC) = 0x801CE404, entry 2
 * (0x801D08C0) = 0x801CF12C, entry 3 (0x801D08C4) = 0x801CF728, entry 4
 * (0x801D08C8) = 0x801CE72C, entry 5 (0x801D08CC) = 0x801CF338, entry 6
 * (0x801D08D0) = 0x801CF954, holding this payload's step handlers. Splat still
 * disassembles the run as the code of the row named 0x801D08B8, so it is only
 * referenced here. This dispatcher indexes it from 0x801D08BC, so its base is
 * entry 1 and the entries it can reach are entries 1 through 6. */
extern void (*D_801D08BC[])(void); /* @source 0x801D08BC @kind data */

/* @source 0x801D07D0
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 * loads the work-record pointer published at the scratchpad pointer cell
 * 0x1F800044 once, takes that record's unsigned halfword at offset 0x2C, scales
 * it by four and calls the matching entry of this overlay's handler run based
 * at 0x801D08BC with no arguments, discarding any result. The original does not
 * bound the index before the scaled table load. It reads no other state and
 * writes none; the pointer-cell load precedes frame creation, the 0x18-byte
 * frame only keeps $ra at 0x10 across the indirect call, and the slots after
 * the index halfword and after the table load stay nops. Neither this function's
 * address nor the table base appears in any word of this payload, so it is an
 * overlay entry reached from outside the payload; its prologue and return path
 * alone place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkHalfword2cTable08bcPlcharPlp05600_801D07D0(void) {
  D_801D08BC[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
