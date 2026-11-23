#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the thirty code pointers from 0x801D0988 through
 * 0x801D09FC, this payload's functions with the shared main-executable handler
 * 0x801B2D54 at entries 2 and 20. The Splat `textbin` row named codePointerTable0988 places
 * all thirty words, so the run is only referenced here. This dispatcher indexes it
 * from 0x801D099C, so its base is entry 5 of that pointer data and the entries it
 * reaches at indices 0 to 24 are 0x801CEC40 through 0x801CFED8, the shared handler
 * 0x801B2D54 among them at index 15. */
extern void (*D_801D099C[])(void); /* @source 0x801D099C @kind data */

/* Seeded from this target's lift at 0x801CE610, which measured exact when this
 * seed was written: the two spans agree word for word except the scaled table
 * load at 0x801CEC20, which names offset 0x99C here and 0x994 there, so the
 * statement order, the shared macro spelling and the declaration style were
 * carried over with this function's own table symbol and name. */
/* @source 0x801CEBFC
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801D099C with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load, so an index
 *           above 24 reads on past the end of the run. It reads no other state
 *           and writes none; the pointer-cell load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slot after the index byte and the slot after the table load stay
 *           nops. Its only in-image reference is the word 0x801CEBFC at
 *           0x801D09EC, entry 25 of the same run, which this dispatcher's own base
 *           reaches at byte-2 index 20; no jal in this payload names the body, so
 *           the overlay loader that steps that run is what calls it and the body
 *           takes no parameters.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTable099Plchar, D_801D099C);
