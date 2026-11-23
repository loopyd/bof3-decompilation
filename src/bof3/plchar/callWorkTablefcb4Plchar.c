#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFCB4: sixteen code pointers, 0x801CFCB4
 * through 0x801CFCF0, holding this payload's own functions plus the shared
 * main-executable handler 0x801B2D54 at entries 2, 7 and 13. The target's
 * `textbin` row of the same base places the words, so the run is only referenced
 * here. This dispatcher indexes it from its first entry, so its base is the run
 * base itself and all sixteen entries are reachable. The run is followed
 * immediately by this payload's last six words at 0x801CFCF4, so the unbounded
 * byte-2 index continues into those at 16 through 21 and reads past the payload
 * end above 21. */
extern void (*codePointerTablefcb4[])(void); /* @source 0x801CFCB4 @kind data */

/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CFCB4 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CE404 at 0x801CFCF4, entry 0 of the payload's last six words, so
 *           the overlay loader that steps that run is what enters it: it has no
 *           jal caller and takes no arguments. The 68-byte body is the same 17
 *           instructions as this overlay's byte-2 dispatchers at 0x801CEE30 and
 *           0x801CFA64 word for word except the low byte of the `lw` offset that
 *           names this table base.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTablefcb4Plchar, codePointerTablefcb4);
