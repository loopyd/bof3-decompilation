#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF5EC: sixteen code pointers, the words
 * 0x801CF5EC through 0x801CF628, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2 and 15. The reviewed
 * Splat layout places all sixteen words in the one `textbin` row at 0x801CF5EC,
 * so the run is only referenced here. This dispatcher indexes the run from its
 * fourth word, so its base is entry 3 = 0x801CE654 and the entries it can reach
 * are entry 3 through entry 15 = 0x801B2D54. */
extern void (*D_801CF5F8[])(void); /* @source 0x801CF5F8 @kind data */

/* Seeded from this target's own lift of the same body at 0x801CE404, which
 * measured exact before this seed was written: the two 68-byte spans agree byte
 * for byte except the low half of the scaled table-load word at 0x801CE634,
 * where each body names the word of the run it indexes. Statement order, the
 * macro spelling and the declaration style were carried over with this
 * function's own table symbol.
 */
/* @source 0x801CE610
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CF5F8 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load, and that load
 *           reads the word at the run's base, so entry 0 of this view is
 *           0x801CE654. It reads no other state and writes none; the
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the indirect call, and the slot after the
 *           index byte and the slot after the table load stay nops. Its only
 *           in-image reference is the word 0x801CE610 at 0x801CF638, entry 3 of
 *           the six-word pointer run at 0x801CF62C that this target's Splat still
 *           declares as code, so the overlay loader that steps that run is what
 *           enters it: it has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchByte2Tablef5f8, D_801CF5F8);
