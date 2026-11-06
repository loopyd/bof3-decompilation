#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0988: thirty code pointers, entry 0 =
 * 0x801CE448 through entry 29 = 0x801CFED8, holding this payload's functions plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 20. The target's
 * Splat `textbin` row of the same base places the words, so the run is only
 * referenced here. This dispatcher indexes it from 0x801D09C4, so its base is
 * entry 15 and the reachable entries are 0x801CF75C through 0x801CFED8. */
extern void (*D_801D09C4[])(void); /* @source 0x801D09C4 @kind data */

/* @source 0x801CF718
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 3, scales it by four and calls
 *           the matching entry of this overlay's handler run based at 0x801D09C4
 *           with no arguments, discarding any result. The original does not bound
 *           the index before the scaled load. It reads no other state and writes
 *           none; the pointer-cell load precedes frame creation, the 0x18-byte
 *           frame only keeps $ra at 0x10 across the indirect call, and the slot
 *           after the index byte and the slot after the table load stay nops. Its
 *           only in-image reference is the word 0x801CF718 at 0x801D09AC, entry 9
 *           of that run, so the byte-2 dispatcher at 0x801CEE28 is what calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3Table09c4PlcharPlp26700_801CF718, D_801D09C4);
