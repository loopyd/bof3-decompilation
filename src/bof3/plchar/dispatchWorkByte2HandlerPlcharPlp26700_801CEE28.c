#include "bof3/emi/dispatchers.h"

/* This overlay's handler table: thirty code pointers, 0x801D0988 through
 * 0x801D09FC, holding this payload's functions plus the shared main-executable
 * handler 0x801B2D54 at entries 2 and 20. The Splat `textbin` row of the same
 * name places the words, so the table is only referenced here. The dispatcher
 * indexes it from 0x801D09A8, so the base is entry 8 and the reachable entries
 * are 0x801CEE6C through 0x801D09FC. */
extern void (*D_801D09A8[])(void); /* @source 0x801D09A8 @kind data */

/* @source 0x801CEE28
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 2, scales it by four and calls
 *           the matching entry of this overlay's handler table based at
 *           0x801D09A8 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled load. It reads no other
 *           state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2HandlerPlcharPlp26700_801CEE28, D_801D09A8);
