#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0630: thirty code pointers, entry 0 =
 * 0x801CE448 through entry 29 = 0x801CF72C, holding this payload's functions plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 8. The Splat
 * `textbin` row of the same name places the words, so the run is only referenced
 * here. This dispatcher indexes it from 0x801D063C, so its base is entry 3 and
 * the entries it can reach inside the run are entries 3 through 29, the words
 * 0x801CE770 through 0x801CF72C, with the shared handler again at entry 8. */
extern void (*D_801D063C[])(void); /* @source 0x801D063C @kind data */

/* @source 0x801CE72C
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801D063C with no arguments,
 * discarding any result. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the slot after the index byte and the slot after the table
 * load stay nops. Its only in-image reference is the word 0x801CE72C at
 * 0x801D069C, entry 27 of the thirty-entry code-pointer run at 0x801D0630, which
 * the byte-2 dispatcher based at or below that entry can reach.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTable063, D_801D063C);
