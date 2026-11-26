#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D02F4: twenty-eight code pointers, entry 0
 * = 0x801CE448 through entry 27 = 0x801D025C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 8 and 21.
 * The Splat `textbin` row codePointerTable02f4 places the words, so the run is only
 * referenced here. This dispatcher indexes it from the interior word 0x801D0300,
 * entry 3, so the twenty-five addresses it can reach are entry 3 = 0x801CE770
 * through entry 27 = 0x801D025C, and its own address is entry 25 of that run. */
extern void (*D_801D0300[])(void); /* @source 0x801D0300 @kind data */

/* Seeded from this payload's 0x801CE404 body
 * (callWorkTable02), whose own lift measured
 * exact. The two 68-byte spans agree in sixteen of seventeen words: the only
 * difference is the low half of the scaled table load, 0x0300 here against
 * 0x02F4 there, because each dispatcher names the base of its own run and both
 * bases sit in the one 0x801D page, so the `lui` of that address stays equal.
 * Statement order, the macro spelling and the declaration style were carried
 * over with this function's own table symbol and name.
 */
/* @source 0x801CE72C
 * @behavior Overlay step dispatch on the work record's unsigned byte at offset
 *           0x02: loads the work-record pointer published at the scratchpad
 *           pointer cell 0x1F800044 once, takes that record's unsigned byte at
 *           offset 2, scales it by four and calls the matching entry of this
 *           overlay's handler run from the interior base 0x801D0300 with no
 *           arguments, discarding any result. The original does not bound the
 *           index before the scaled table load. It reads no other state and
 *           writes none; the pointer-cell load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slots after the index byte and after the table load stay nops.
 *           Its address appears once in the payload, as the word 0x801CE72C at
 *           0x801D0358, entry 25 of the twenty-eight-word run and index 22 of the
 *           run based at 0x801D0300, so no jal in this payload names it; the
 *           sibling halfword dispatcher
 *           callWorkHalfword2Table0358 indexes that
 *           interior word with the work halfword at offset 0x2C and so reaches
 *           this body as its entry 0, while the overlays enter the body itself
 *           from outside the payload. It takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTable0300, callWorkTable0300, D_801D0300);
