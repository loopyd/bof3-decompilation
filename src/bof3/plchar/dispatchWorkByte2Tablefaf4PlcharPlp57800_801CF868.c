#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAB8: twenty-three code pointers,
 * 0x801CFAB8 through 0x801CFB10, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 8 and 14. The run
 * crosses the target's `textbin T_801CFAB8` row boundary at 0x801CFAF8 and ends
 * with the payload, so it is only referenced here. This dispatcher indexes it
 * from 0x801CFAF4, entry 15, the last word the `T_801CFAB8` row holds, so the
 * reachable entries are 0x801CF8AC, 0x801CF9C4, 0x801CE404, 0x801CEA00,
 * 0x801CF65C, 0x801CE610, 0x801CEC34 and this body itself at 0x801CF868; an
 * index above 7 leaves the payload. */
extern void (*D_801CFAF4[])(void); /* @source 0x801CFAF4 @kind data */

/* @source 0x801CF868
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run indexed from
 *           0x801CFAF4 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. No jal of this payload targets the body.
 *           Its address appears once in the payload, as the word 0x801CF868 at
 *           0x801CFB10, entry 22 of the run based at 0x801CFAB8, so the overlay
 *           loader enters the body from outside the payload while the body also
 *           reaches itself as index 7 of its own table.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tablefaf4PlcharPlp57800_801CF868, D_801CFAF4);
