#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the thirty code pointers from 0x801D0988 through
 * 0x801D09FC, this payload's functions with the shared main-executable handler
 * 0x801B2D54 at entries 2 and 20. The Splat `textbin` row of the same name
 * places all thirty words, so the run is only referenced here. This dispatcher
 * indexes it from 0x801D0994, so its base is entry 3 of that pointer data and
 * the entries it reaches at indices 0 to 26 are 0x801CE654 through 0x801CFED8,
 * the shared handler 0x801B2D54 among them at index 17. */
extern void (*D_801D0994[])(void); /* @source 0x801D0994 @kind data */

/* Seeded from the lift of the same body at 0x801CE610 of plp567, whose handler
 * run sits at 0x801D0798 and so in the same 0x801D page. Comparing this target's
 * payload with that one shows the two 68-byte spans agree in every word except
 * the scaled table load at 0x801CE634, which names offset 0x798 there and 0x994
 * here, so the statement order, the macro spelling and the declaration style
 * were carried over with this function's own table symbol and name.
 */
/* @source 0x801CE610
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801D0994 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load, so an index
 *           above 26 reads on past the end of the run. It reads no other state
 *           and writes none; the pointer-cell load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slots after the index byte and after the table load stay nops.
 *           Its only in-image reference is the word 0x801CE610 at 0x801D09F4,
 *           entry 27 of the same run, which this dispatcher's own base reaches
 *           at byte-2 index 24; no jal in this payload names the body, so the
 *           overlay loader that steps that run is what calls it and the body
 *           takes no parameters.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTable0994, callWorkTable0994, D_801D0994);
