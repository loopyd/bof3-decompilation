#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFD80: sixteen code pointers, the words
 * 0x801CFD80 through 0x801CFDBC, entry 0 = 0x801CE448 through entry 15 =
 * 0x801CF938, holding this payload's own functions plus the shared
 * main-executable handler 0x801B2D54 at entries 2, 8 and 13. The reviewed Splat
 * layout places all sixteen words in the one `textbin` row at 0x801CFD80, so the
 * run is only referenced here. This dispatcher indexes it from 0x801CFDAC, entry
 * 11 of the run, so the entries it can reach from that base are 0x801CF544,
 * 0x801CF610, 0x801B2D54, 0x801CF750 and 0x801CF938. */
extern void (*D_801CFDAC[])(void); /* @source 0x801CFDAC @kind data */

/* Seeded from the four other copies of this body in the payload, at 0x801CE404,
 * 0x801CE72C, 0x801CF12C and 0x801CF338, all of which measure exact. A `cmp` of
 * those 68-byte spans against this one leaves the word at byte 36 alone
 * differing, the low half of the scaled table load at 0x801CF524, 0xAC here
 * against 0x80, 0x8C, 0x98 and 0xA4 there; each copy names the word of the run it
 * indexes, and all five bases share the one 0x8000 page, so that immediate is the
 * only differing word. Because the reference differs, those siblings are shape
 * evidence and not a clone: statement order, the macro spelling and the
 * declaration style were carried over with this function's own table symbol.
 */
/* @source 0x801CF500
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CFDAC with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load, and that base
 *           is entry 11 of the sixteen-word handler run at 0x801CFD80, so index 0
 *           reaches 0x801CF544. It reads no other state and writes none; the
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the indirect call, and the slot after the
 *           index byte and the slot after the table load stay nops. No jal in
 *           this payload names it: the loader enters it through the word
 *           0x801CF500 at 0x801CFDC8, entry 2 of the six-word pointer run at
 *           0x801CFDC0 that this target's Splat declares as a `textbin` row and
 *           that the sibling halfword-0x2C dispatcher at 0x801CFCF8 indexes from
 *           that run's base, so it takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTablefdac,
                      callWorkTablefdac,
                      D_801CFDAC);
