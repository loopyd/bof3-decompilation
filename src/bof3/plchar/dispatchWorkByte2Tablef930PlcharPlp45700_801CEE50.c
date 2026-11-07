#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF900: twenty-one code pointers, the words
 * 0x801CF900 through 0x801CF950, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 5 and 11. The leading
 * sixteen words sit inside the 0x801CF900 `textbin` row and the trailing five in
 * the 0x801CF940 row, which the reviewed Splat layout places, so the run is only
 * referenced here. This dispatcher indexes it from 0x801CF930, so its base is
 * entry 12 and the reachable entries are 0x801CEE94 through 0x801CEE50. */
extern void (*D_801CF930[])(void); /* @source 0x801CF930 @kind data */

/* @source 0x801CEE50
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801CF930 with no arguments,
 * discarding any result. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the slot after the index byte and the slot after the table
 * load stay nops. Its only in-image reference is the word 0x801CEE50 at
 * 0x801CF950, entry 20 of the twenty-one-entry code-pointer run at 0x801CF900,
 * which this dispatcher's own run reaches at byte-2 index 8, so the overlay
 * loader that steps that run is what calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tablef930PlcharPlp45700_801CEE50, D_801CF930);
