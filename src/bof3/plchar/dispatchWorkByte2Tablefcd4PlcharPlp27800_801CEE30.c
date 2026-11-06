#include "bof3/emi/dispatchers.h"

/* This overlay's trailing handler run at 0x801CFCB4: sixteen code pointers,
 * 0x801CFCB4 through 0x801CFCF0, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 7 and 13. The target's
 * `textbin` row of the same base places the words, so the run is only referenced
 * here. This dispatcher indexes it from 0x801CFCD4, so its base is entry 8 and
 * the reachable entries are 0x801CEE74 through 0x801CFBC0. */
extern void (*D_801CFCD4[])(void); /* @source 0x801CFCD4 @kind data */

/* @source 0x801CEE30
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 2, scales it by four and calls
 *           the matching entry of this overlay's handler run based at 0x801CFCD4
 *           with no arguments, discarding any result. The original does not bound
 *           the index before the scaled load. It reads no other state and writes
 *           none; the pointer-cell load precedes frame creation, the 0x18-byte
 *           frame only keeps $ra at 0x10 across the indirect call, and the slot
 *           after the index byte and the slot after the table load stay nops.
 *           This payload holds no in-image reference to the function, neither a
 *           table word nor a call, so the overlay loader outside the payload is
 *           what enters it; its own prologue and return path bound the range.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tablefcd4PlcharPlp27800_801CEE30, D_801CFCD4);
