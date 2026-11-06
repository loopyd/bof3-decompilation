#include "bof3/emi/dispatchers.h"

/* This overlay's handler table: thirty code pointers, 0x801D0564 through
 * 0x801D05D8, holding this payload's functions plus the shared main-executable
 * handler 0x801B2D54 at entries 15 and 21. The Splat `textbin` row T_801D0564
 * places the words, so the table is only referenced here. This dispatcher
 * indexes it from 0x801D0570, so the base is entry 3 and the reachable entries
 * are 0x801D0570 through 0x801D05D8. */
extern void (*D_801D0570[])(void); /* @source 0x801D0570 @kind data */

/* @source 0x801CE630
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 2, scales it by four and calls
 *           the matching entry of this overlay's handler table based at
 *           0x801D0570 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled load. It reads no other
 *           state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Table0570PlcharPlp67800_801CE630, D_801D0570);
