#include "bof3/emi/dispatchers.h"

/* This overlay's handler table: thirty code pointers, 0x801D0564 through
 * 0x801D05D8, holding this payload's functions plus the shared main-executable
 * handler 0x801B2D54 at entries 15 and 21. The Splat `textbin` row T_801D0564
 * places the words, so the table is only referenced here. This dispatcher
 * indexes it from 0x801D05BC, so the base is entry 22 and the reachable entries
 * are 0x801D05BC through 0x801D05D8. */
extern void (*D_801D05BC[])(void); /* @source 0x801D05BC @kind data */

/* @source 0x801D0314
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 2, scales it by four and calls
 *           the matching entry of this overlay's handler table based at
 *           0x801D05BC with no arguments, discarding any result. The original
 *           does not bound the index before the scaled load. It reads no other
 *           state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801D0314 at 0x801D05D8, entry 29 of the thirty-entry code-pointer
 *           run at 0x801D0564, which this dispatcher's own base reaches at index
 *           7.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Table05bcPlcharPlp67800_801D0314, D_801D05BC);
