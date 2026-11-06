#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D02F4: twenty-eight code pointers, entry 0
 * = 0x801CE448 through entry 27 = 0x801D025C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 8 and 21.
 * The Splat `textbin` row T_801D02F4 places the words, so the run is only
 * referenced here. This dispatcher indexes it from 0x801D0320, so its base is
 * entry 11 and the reachable entries are 0x801CF3C0 through 0x801D025C. Its own
 * address is entry 9 of the same run, the word 0x801CF37C at 0x801D0318, and
 * that word is its only in-image reference, so func_801CE404 steps the run on
 * work-record byte 2 and calls it from there. */
extern void (*D_801D0320[])(void); /* @source 0x801D0320 @kind data */

/* @source 0x801CF37C
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 3, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801D0320 with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call, and the slot after the index byte and the slot
 * after the table load stay nops. It has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3Table0320PlcharPlp03400_801CF37C, D_801D0320);
