#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFD80: sixteen code pointers, the words
 * 0x801CFD80 through 0x801CFDBC, entry 0 = 0x801CE448 through entry 15 =
 * 0x801CF938, holding this payload's own functions plus the shared
 * main-executable handler 0x801B2D54 at entries 2, 8 and 13. The reviewed Splat
 * layout places all sixteen words in the one `textbin` row at 0x801CFD80, so the
 * run is only referenced here. This dispatcher indexes it from 0x801CFDB8, entry
 * 14 of the run, so the eight-word window it can reach leaves that row after its
 * first two words and continues through the whole six-word `textbin` row at
 * 0x801CFDC0: 0x801CF750, 0x801CF938, 0x801CE404, 0x801CF12C, 0x801CF500,
 * 0x801CE72C, 0x801CF338 and 0x801CF70C itself. */
extern void (*D_801CFDB8[])(void); /* @source 0x801CFDB8 @kind data */

/* Seeded from the five other copies of this body in the payload, at 0x801CE404,
 * 0x801CE72C, 0x801CF12C, 0x801CF338 and 0x801CF500, all of which measure exact.
 * A `cmp` of those 68-byte spans against this one leaves the word at byte 36
 * alone differing, the low half of the scaled table load at 0x801CF730, 0xB8 here
 * against 0x80, 0x8C, 0x98, 0xA4 and 0xAC there; each copy names the word of the
 * run it indexes, and all six bases share the one 0x8000 page, so that immediate
 * is the only differing word. Because the reference differs, those siblings are
 * shape evidence and not a clone: statement order, the macro spelling and the
 * declaration style were carried over with this function's own table symbol.
 */
/* @source 0x801CF70C
 * @behavior Overlay step dispatch on the work-record byte 2, the work step
 *           counter: loads the work-record pointer published at the scratchpad
 *           pointer cell 0x1F800044 once, takes that record's unsigned byte at
 *           offset 2, scales it by four and calls the matching entry of this
 *           overlay's eight-word handler run based at 0x801CFDB8 with no
 *           arguments, discarding any result. The original does not bound the
 *           index before the scaled table load. That base is entry 14 of the
 *           sixteen-word handler run at 0x801CFD80, so index 0 reaches
 *           0x801CF750 and the eight entries the window can reach cross into the
 *           next reviewed row: 0x801CF750, 0x801CF938, 0x801CE404, 0x801CF12C,
 *           0x801CF500, 0x801CE72C, 0x801CF338 and 0x801CF70C. It reads no other
 *           state and writes none; the pointer-cell load precedes frame creation,
 *           the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 *           and the slot after the index byte and the slot after the table load
 *           stay nops. No jal in this payload names it: the loader enters it
 *           through the word 0x801CF70C at 0x801CFDD4, entry 5 of the six-word
 *           pointer run at 0x801CFDC0 that this target's Splat declares as a
 *           `textbin` row and that the sibling halfword-0x2C dispatcher at
 *           0x801CFCF8 indexes from that run's base, so it takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTablefdb8,
                      callWorkTablefdb8,
                      D_801CFDB8);
