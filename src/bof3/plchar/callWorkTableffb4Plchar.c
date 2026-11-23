#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFFA8: sixteen code pointers, the words
 * 0x801CFFA8 through 0x801CFFE4, holding this payload's step handlers plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 8 and 13. The reviewed
 * Splat layout places those sixteen words in the one `textbin` row at 0x801CFFA8
 * and the seven words after them in the `textbin` row at 0x801CFFE8, so the run
 * is only referenced here. This dispatcher indexes it from the interior word
 * 0x801CFFB4, entry 3, so the sixteen addresses it can reach are entry 0 =
 * 0x801CE770 through entry 15 = 0x801CF12C, and the last three of those words sit
 * in the 0x801CFFE8 row. */
extern void (*D_801CFFB4[])(void); /* @source 0x801CFFB4 @kind data */

/* Seeded from the lift of the same body at this payload's 0x801CE404
 * (callWorkTableffa8Plchar), which is exact. The two
 * 68-byte spans agree in sixteen of seventeen words: the only difference is the
 * low half of the scaled table load, 0xFFB4 here against 0xFFA8 there, because
 * each dispatcher names the base of its own run and both bases sit in the one
 * 0x801D page. Statement order, the macro spelling and the declaration style
 * were carried over with this function's own table symbol and name.
 */
/* @source 0x801CE72C
 * @behavior Overlay step dispatch on the work record's unsigned byte at offset
 *           0x02: loads the work-record pointer published at the scratchpad
 *           pointer cell 0x1F800044 once, takes that record's unsigned byte at
 *           offset 2, scales it by four and calls the matching entry of this
 *           overlay's handler run from the interior base 0x801CFFB4 with no
 *           arguments, discarding any result. The original does not bound the
 *           index before the scaled table load. It reads no other state and
 *           writes none; the pointer-cell load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slots after the index byte and after the table load stay nops.
 *           The word 0x801CE72C sits at 0x801CFFF8, entry 4 of the seven-word
 *           pointer run at 0x801CFFE8, and that interior word is the base the
 *           sibling dispatcher at 0x801CFF64 indexes, so the work halfword at
 *           offset 0x2C selects this function at index zero. It has no jal
 *           caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTableffb4Plchar, D_801CFFB4);
