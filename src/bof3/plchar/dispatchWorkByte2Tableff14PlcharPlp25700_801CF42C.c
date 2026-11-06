#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFEDC: twenty-three code pointers, entry 0
 * = 0x801CE448 through entry 22 = 0x801CF42C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 7 and 13. The
 * target's Splat `textbin` row places entries 0 through 15 and the following row
 * carries the rest of the run, so the run is only referenced here. This
 * dispatcher indexes it from 0x801CFF14, so its base is entry 14 and the
 * reachable entries are 0x801CF470 through 0x801CF42C. */
extern void (*D_801CFF14[])(void); /* @source 0x801CFF14 @kind data */

/* @source 0x801CF42C
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801CFF14 with no arguments,
 * discarding any result. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the slot after the index byte and the slot after the table
 * load stay nops. Its only in-image reference is the word 0x801CF42C at
 * 0x801CFF34, entry 22 of the twenty-three-entry code-pointer run at
 * 0x801CFEDC, which both the byte-2 dispatcher based at 0x801CFF08 (index 11)
 * and this dispatcher (index 8) reach, so the overlay loader that steps that run
 * is what calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tableff14PlcharPlp25700_801CF42C, D_801CFF14);
