#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFFA8: sixteen code pointers, the words
 * 0x801CFFA8 through 0x801CFFE4, holding this payload's step handlers plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 8 and 13. The reviewed
 * Splat layout places those sixteen words in the one `textbin` row at 0x801CFFA8
 * and the seven words after them in the `textbin` row at 0x801CFFE8, so the run
 * is only referenced here. This dispatcher indexes the run from the interior word
 * 0x801CFFCC, entry 9, so the fourteen addresses it can reach on index values
 * 0 through 13 are 0x801CF37C through 0x801CFB30, and index values 14 and 15
 * scale onto words past the payload's end at 0x801D0004. */
extern void (*D_801CFFCC[])(void); /* @source 0x801CFFCC @kind data */

/* Seeded from the lift of the same body at this payload's 0x801CF12C
 * (callWorkTableffc0Plchar), whose metadata recorded
 * it exact. The two 68-byte spans agree in sixteen of seventeen words: the only
 * difference is the low half of the scaled table load at 0x801CF35C, 0xFFCC here
 * against 0xFFC0 there, because each dispatcher names the interior base of its own
 * index window into the one run at 0x801CFFA8 and both bases sit in the one
 * 0x801D page. Statement order, the macro spelling and the declaration style were
 * carried over with this function's own table symbol and name.
 */
/* @source 0x801CF338
 * @behavior Overlay step dispatch on the work record's unsigned byte at offset
 *           0x02: loads the work-record pointer published at the scratchpad
 *           pointer cell 0x1F800044 once, takes that record's unsigned byte at
 *           offset 2, scales it by four and calls the matching entry of this
 *           overlay's handler run from the interior base 0x801CFFCC with no
 *           arguments, discarding any result. The original does not bound the
 *           index before the scaled table load. It reads no other state and
 *           writes none; the pointer-cell load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slots after the index byte and after the table load stay nops.
 *           The word 0x801CFFFC holds this function, index 12 of the interior
 *           base 0x801CFFCC and entry 6 of the seven-word pointer run at
 *           0x801CFFE8. It has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTableffccPlchar, D_801CFFCC);
