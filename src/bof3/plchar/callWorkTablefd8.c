#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFD80: sixteen code pointers, the words
 * 0x801CFD80 through 0x801CFDBC, entry 0 = 0x801CE448 through entry 15 =
 * 0x801CF938, holding this payload's own functions plus the shared
 * main-executable handler 0x801B2D54 at entries 2, 8 and 13. The reviewed Splat
 * layout places all sixteen words in the one `textbin` row at 0x801CFD80, so the
 * run is only referenced here. This dispatcher indexes it from 0x801CFD8C, entry
 * 3 of the run, so the entries it can reach from that base are 0x801CE770,
 * 0x801CEA94, 0x801CEC30, 0x801CF170, 0x801CF23C, 0x801B2D54, 0x801CF37C,
 * 0x801CF494, 0x801CF500, 0x801CF610, 0x801B2D54, 0x801CF750 and 0x801CF938. */
extern void (*D_801CFD8C[])(void); /* @source 0x801CFD8C @kind data */

/* Seeded from the overlay lift of the same body at 0x801CE404, whose own row
 * indexes the base of this run. Its 68 bytes measured exact, and the two spans
 * agree byte for byte except the low half of the scaled table-load word at
 * 0x801CE750, where each body names the word of the run it indexes. Because that
 * word differs, the sibling is shape evidence only and not a clone: statement
 * order, the macro spelling and the declaration style were carried over with this
 * function's own table symbol.
 */
/* @source 0x801CE72C
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CFD8C with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load, and that base
 *           is entry 3 of the sixteen-word handler run at 0x801CFD80, so index 0
 *           reaches 0x801CE770. It reads no other state and writes none; the
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the indirect call, and the slot after the
 *           index byte and the slot after the table load stay nops. No jal in
 *           this payload names it: the loader enters it through the word
 *           0x801CE72C at 0x801CFDCC, entry 3 of the six-word pointer run at
 *           0x801CFDC0, which the sibling halfword-0x2C dispatcher at 0x801CFD3C
 *           indexes, so it takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTablefd8,
                      callWorkTablefd8,
                      D_801CFD8C);
