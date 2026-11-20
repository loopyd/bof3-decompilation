#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFFA8: sixteen code pointers, the words
 * 0x801CFFA8 through 0x801CFFE4, holding this payload's step handlers plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 8 and 13, followed by
 * the seven pointers of the run at 0x801CFFE8. The reviewed Splat layout places
 * those words in the two `textbin` rows at 0x801CFFA8 and 0x801CFFE8, so the run
 * is only referenced here. This dispatcher indexes it from the interior word
 * 0x801CFFE0, entry 14 of the sixteen-word row at 0x801CFFA8, so the nine
 * addresses its index values 0 through 8 reach are 0x801CFB74, 0x801CFD40,
 * 0x801CFE4C, 0x801CE404, 0x801CF12C, 0x801CF924, 0x801CE72C, 0x801CF338 and
 * 0x801CFB30, the last of them this function itself, and index values 9 and
 * above scale onto words at or past 0x801D0004, beyond the payload's end. */
extern void (*D_801CFFE0[])(void); /* @source 0x801CFFE0 @kind data */

/* Seeded from the lift of the same body at this payload's 0x801CF924
 * (dispatchWorkByte2Tableffd4PlcharPlp02500_801CF924), whose metadata recorded
 * it exact. The two 68-byte spans agree in sixteen of seventeen words: the only
 * difference is the low half of the scaled table load at 0x801CFB54, 0xFFE0 here
 * against 0xFFD4 there, because each dispatcher names the interior base of its own
 * index window into the one pointer run based at 0x801CFFA8 and both bases sit in
 * the one 0x801D page. Statement order, the macro spelling and the declaration
 * style were carried over with this function's own table symbol and name. A scan
 * of the 398 extracted payloads under out/binaries found these 68 bytes only in
 * this payload at offset 0x1730, so no byte-identical twin exists and this
 * in-payload exact sibling is the seed.
 */
/* @source 0x801CFB30
 * @behavior Overlay step dispatch on the work record's unsigned byte at offset
 *           0x02: loads the work-record pointer published at the scratchpad
 *           pointer cell 0x1F800044 once, takes that record's unsigned byte at
 *           offset 2, scales it by four and calls the matching entry of this
 *           overlay's handler run from the interior base 0x801CFFE0 with no
 *           arguments, discarding any result. The original does not bound the
 *           index before the scaled table load. It reads no other state and
 *           writes none; the pointer-cell load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slots after the index byte and after the table load stay nops.
 *           The word 0x801D0000, the last word of the payload, holds this
 *           function, index 8 of the interior base 0x801CFFE0 and entry 6 of the
 *           seven-word pointer run at 0x801CFFE8. It has no jal caller and takes
 *           no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tableffe0PlcharPlp02500_801CFB30, D_801CFFE0);
