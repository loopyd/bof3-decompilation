#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D02F4: twenty-eight code pointers, entry 0
 * = 0x801CE448 through entry 27 = 0x801D025C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 8 and 21.
 * The Splat `textbin` row T_801D02F4 places the words, so the run is only
 * referenced here. This dispatcher indexes it from 0x801D0340, so its base is
 * entry 19 and its reachable entries are the nine words 0x801D0094, 0x801D0160,
 * 0x801B2D54, 0x801CE404, 0x801CF12C, 0x801D0050, 0x801CE72C, 0x801CF338 and
 * 0x801D025C through the end of the run. Its own address is entry 5 of that
 * nine-word tail, the word 0x801D0050 at 0x801D0354, and that word is its only
 * in-image reference, so the overlays enter it through this table. */
extern void (*D_801D0340[])(void); /* @source 0x801D0340 @kind data */

/* @source 0x801D0050
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801D0340 with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call, and the slot after the index byte and the slot
 * after the table load stay nops. It has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Table0340PlcharPlp03400_801D0050, D_801D0340);
