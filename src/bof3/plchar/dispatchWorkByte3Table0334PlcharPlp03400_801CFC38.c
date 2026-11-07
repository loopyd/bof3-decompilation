#include "bof3/emi/dispatchers.h"

/* Second work-record step dispatcher of this overlay. The Splat `textbin` row
 * T_801D02F4 places the overlay's handler run, twenty-eight code pointers from
 * 0x801D02F4 through 0x801D0358. This dispatcher indexes it from 0x801D0334,
 * which is entry 16 of that run, so its reachable entries start at 0x801CFC7C
 * (the following function) and continue to 0x801D025C. It reads the same
 * scratchpad pointer cell as the overlay dispatcher at 0x801CF37C but a
 * different base inside the one run, so both are kept as separate functions. */
extern void (*D_801D0334[])(void); /* @source 0x801D0334 @kind data */

/* @source 0x801CFC38
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 3, scales it by four and calls the matching
 * entry of the handler run based at 0x801D0334 with no arguments, discarding any
 * result. The original does not bound the index before the scaled table load. It
 * reads no other state and writes none; the pointer-cell load precedes frame
 * creation, the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 * and the slot after the index byte and the slot after the table load stay nops.
 * It has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3Table0334PlcharPlp03400_801CFC38, D_801D0334);
