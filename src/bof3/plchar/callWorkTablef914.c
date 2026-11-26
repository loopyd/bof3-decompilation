#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF900: seventeen code pointers, the words
 * 0x801CF900 through 0x801CF940, holding this payload's own step handlers plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 7. The reviewed
 * Splat layout places all seventeen words in the one `textbin` row at 0x801CF900,
 * so the run is only referenced here. This dispatcher indexes it from 0x801CF914,
 * so its base is entry 5 of that run and entry 0 is 0x801CEC40. */
extern void (*D_801CF914[])(void); /* @source 0x801CF914 @kind data */

/* Seeded from the exact lift of the same body in this target at 0x801CE404
 * (callWorkTablef900): the two 68-byte spans
 * differ in one word only, the table load's `%lo` immediate, 0xF900 there and
 * 0xF914 here, and both table words sit in the one 0x801D page, so the page
 * halves and every other word stay equal. The other dispatcher in this target,
 * 0x801CE610, shows the same single-word difference at its own base 0x801CF90C.
 * No payload span in this repository's binary set is byte-identical to this one,
 * so the body was transcribed rather than cloned. Statement order, the macro
 * spelling and the declaration style carry over unchanged; this file writes its
 * own emitted name and its own table base. */
/* @source 0x801CEBFC
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CF914 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CEBFC at 0x801CF930, entry 12 of the seventeen-word code-pointer
 *           run at 0x801CF900, which this payload's work-halfword 0x2C
 *           dispatcher at 0x801CF878 indexes from its base 0x801CF92C (entry
 *           11), so this body is reached when that halfword reads 1. It has no
 *           jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTablef914, D_801CF914);
