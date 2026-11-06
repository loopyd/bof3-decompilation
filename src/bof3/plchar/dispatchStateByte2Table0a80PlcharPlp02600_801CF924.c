#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0A54: thirty code pointers, the words
 * 0x801D0A54 through 0x801D0AC8, holding this payload's functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2 and 8. The Splat
 * `textbin` row of the same name places the words, so the run is only
 * referenced here. This dispatcher indexes it from 0x801D0A80, so its base is
 * entry 11 and the reachable entries are 0x801CF968 through 0x801CFB50. Its own
 * address is entry 26 of the same run, the word 0x801CF924 at 0x801D0ABC, and
 * that word is its only in-image reference, so the overlay loader that steps
 * the run is what calls it. */
extern void (*D_801D0A80[])(void); /* @source 0x801D0A80 @kind data */

/* @source 0x801CF924
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801D0A80 with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call, and the slot after the index byte and the slot
 * after the table load stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table0a80PlcharPlp02600_801CF924, D_801D0A80);
