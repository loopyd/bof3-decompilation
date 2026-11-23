#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAB8: twenty-three code pointers, 0x801CFAB8
 * through 0x801CFB10, holding this payload's own functions plus the shared
 * main-executable handler 0x801B2D54 at entries 2, 8 and 14. The run fills the
 * target's two `textbin` rows at 0x801CFAB8 and 0x801CFAF8, so it is only
 * referenced here. This dispatcher indexes it from 0x801CFB08, so its base is
 * entry 20 of that run and the three entries it reaches are 0x801CE610,
 * 0x801CEC34 and 0x801CF868, the byte-2 dispatchers of this payload. The
 * neighbouring base 0x801CFAFC, indexed by the dispatcher at 0x801CFA30, starts
 * six entries earlier and holds those three as its last three. */
extern void (*D_801CFB08[])(void); /* @source 0x801CFB08 @kind data */

/* @source 0x801CFA74
 * @behavior Overlay step dispatch on the work record's halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFB08 with no arguments,
 *           discarding any result. The original does not bound the index before
 *           the scaled table load. It reads no other state and writes none; the
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the indirect call, and the slots after the
 *           index halfword and after the table load stay nops. Neither this
 *           function's address nor the table base appears in any word of this
 *           payload and no jal names it, so it is an overlay entry reached from
 *           outside the payload; its prologue and return path alone place the
 *           boundary. The 68-byte body is the same seventeen instructions as the
 *           dispatcher at 0x801CFA30 word for word except the `lw` offset that
 *           names this table base, which differs by 0xC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkHalfword2Tablefb08(void) {
  D_801CFB08[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
