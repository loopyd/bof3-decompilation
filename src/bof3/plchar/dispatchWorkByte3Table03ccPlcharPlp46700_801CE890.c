#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D03AC: twenty-eight code pointers,
 * 0x801D03AC through 0x801D0418, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2 and 18. The Splat
 * `textbin` row T_801D03AC places the words, so the run is only referenced here.
 * This dispatcher indexes it from 0x801D03CC, so its base is entry 8 of that run
 * and index 0 of that base is 0x801CE8D4. */
extern void (*D_801D03CC[])(void); /* @source 0x801D03CC @kind data */

/* @source 0x801CE890
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 3, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801D03CC with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps the return address at 0x10
 *           across the indirect call, and the slot after the index byte and the
 *           slot after the table load stay nops. Its only in-image reference is
 *           the word at 0x801D03C4, index 0 of the handler run based at
 *           0x801D03C4 and entry 6 of the run based at 0x801D03AC, so the
 *           work-byte-2 dispatcher with that base enters it: it has no jal caller
 *           and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3Table03ccPlcharPlp46700_801CE890, D_801D03CC);
