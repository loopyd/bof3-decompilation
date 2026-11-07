#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAB8: twenty-three code pointers,
 * 0x801CFAB8 through 0x801CFB10, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 8 and 14. The run
 * crosses the target's `textbin func_801CFAB8` row boundary at 0x801CFAF8 and
 * stops one word before the payload does, so it is only referenced here. This
 * dispatcher indexes it from 0x801CFAE8, so its base is entry 12 and the
 * reachable entries are 0x801CF6A0, 0x801CF76C, 0x801B2D54, 0x801CF8AC,
 * 0x801CF9C4, 0x801CE404, 0x801CEA00, 0x801CF65C, 0x801CE610, 0x801CEC34 and
 * 0x801CF868. */
extern void (*D_801CFAE8[])(void); /* @source 0x801CFAE8 @kind data */

/* @source 0x801CF65C
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CFAE8 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CF65C at 0x801CFB04, entry 19 of the run based at 0x801CFAB8, so
 *           the loader that steps that run enters it: it has no jal caller and
 *           takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tablefae8PlcharPlp57800_801CF65C, D_801CFAE8);
