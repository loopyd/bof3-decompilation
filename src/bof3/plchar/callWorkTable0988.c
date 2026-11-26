#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0988: thirty code pointers, entry 0 =
 * 0x801CE448 through entry 29 = 0x801CFED8, holding this payload's functions plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 20. The target's
 * Splat `textbin` row of the same name places the words, so the run is only
 * referenced here. This dispatcher indexes it from its first word, so its base is
 * entry 0 and the entries it can reach are entry 0 = 0x801CE448 through entry 29
 * = 0x801CFED8. */
extern void (*codePointerTable0988[])(void); /* @source 0x801D0988 @kind data */

/* Seeded from this target's own exact byte-2 dispatchers, carried by the shared
 * macro in include/bof3/emi/dispatchers.h: the 68 bytes at offset 4 agree word for
 * word with the span at 0x801CEE28 except the scaled table load, where each body
 * names its own run, so only the table symbol and the name are this function's.
 */
/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 2, scales it by four and calls
 *           the matching entry of this overlay's handler run based at 0x801D0988
 *           with no arguments, discarding any result. The original does not bound
 *           the index before the scaled table load. It reads no other state and
 *           writes none; the pointer-cell load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slot after the index byte and the slot after the table load stay
 *           nops. Its only in-image reference is the word 0x801CE404 at
 *           0x801D09E8, entry 24 of the same thirty-entry run, which this
 *           dispatcher's own base reaches at byte-2 index 24; no jal in this
 *           payload names the body, so the overlay loader that steps that run is
 *           what calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTable0988, callWorkTable0988, codePointerTable0988);
