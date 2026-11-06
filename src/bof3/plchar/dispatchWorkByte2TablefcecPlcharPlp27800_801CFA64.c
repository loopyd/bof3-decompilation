#include "bof3/emi/dispatchers.h"

/* This overlay's trailing handler run at 0x801CFCB4: sixteen code pointers,
 * 0x801CFCB4 through 0x801CFCF0, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 7 and 13. The target's
 * `textbin` row of the same base places the words, so the run is only referenced
 * here. This dispatcher indexes it from 0x801CFCEC, so its base is entry 14 of
 * that run and the entries the run itself holds from there are 0x801CFAA8 and
 * 0x801CFBC0. */
extern void (*D_801CFCEC[])(void); /* @source 0x801CFCEC @kind data */

/* @source 0x801CFA64
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CFCEC with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CFA64 at 0x801CFD08, entry 5 of the pointer run based at
 *           0x801CFCF4, so the loader that steps that run enters it: it has no
 *           jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2TablefcecPlcharPlp27800_801CFA64, D_801CFCEC);
