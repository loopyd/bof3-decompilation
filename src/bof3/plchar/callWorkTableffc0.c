#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFFA8: sixteen code pointers, the words
 * 0x801CFFA8 through 0x801CFFE4, holding this payload's step handlers plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 8 and 13. The reviewed
 * Splat layout places those sixteen words in the one `textbin` row at 0x801CFFA8
 * and the seven words after them in the `textbin` row at 0x801CFFE8, so the run
 * is only referenced here. This dispatcher indexes the run from the interior word
 * 0x801CFFC0, entry 6, so the sixteen addresses it can reach are entry 0 =
 * 0x801CF170 through entry 15 = 0x801CF338, and the last six of those words sit
 * in the 0x801CFFE8 row. */
extern void (*D_801CFFC0[])(void); /* @source 0x801CFFC0 @kind data */

/* Seeded from the lift of the same body at this payload's 0x801CE72C
 * (callWorkTableffb4), whose metadata recorded
 * it exact. The two 68-byte spans agree in sixteen of seventeen words: the only
 * difference is the low half of the scaled table load at 0x801CF154, 0xFFC0 here
 * against 0xFFB4 there, because each dispatcher names the interior base of its own
 * index window into the one run at 0x801CFFA8 and both bases sit in the one
 * 0x801D page. Statement order, the macro spelling and the declaration style were
 * carried over with this function's own table symbol and name.
 */
/* @source 0x801CF12C
 * @behavior Overlay step dispatch on the work record's unsigned byte at offset
 *           0x02: loads the work-record pointer published at the scratchpad
 *           pointer cell 0x1F800044 once, takes that record's unsigned byte at
 *           offset 2, scales it by four and calls the matching entry of this
 *           overlay's handler run from the interior base 0x801CFFC0 with no
 *           arguments, discarding any result. The original does not bound the
 *           index before the scaled table load. It reads no other state and
 *           writes none; the pointer-cell load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slots after the index byte and after the table load stay nops.
 *           The word 0x801CFFF0 holds this function, entry 2 of the seven-word
 *           pointer run at 0x801CFFE8. That word is also index 1 of the interior
 *           base 0x801CFFEC that the work-halfword 0x2C dispatcher at 0x801CFF20
 *           indexes, so that dispatcher selects this function when the work
 *           halfword at offset 0x2C reads 1. It has no jal caller and takes no
 *           arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTableffc0, D_801CFFC0);
