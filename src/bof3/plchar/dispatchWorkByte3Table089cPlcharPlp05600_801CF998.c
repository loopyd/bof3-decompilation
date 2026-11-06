#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0858: twenty-four code pointers, entry 0
 * = 0x801CE448 through entry 23 = 0x801D02F4, holding this payload's
 * functions plus the shared main-executable handler 0x801B2D54 at entries 2 and
 * 8. The Splat `textbin` row of the same name places the words, so the run is
 * only referenced here. This dispatcher indexes it from 0x801D089C, so its base
 * is entry 17 and the reachable entries are 0x801CF9DC through 0x801D02F4. */
extern void (*D_801D089C[])(void); /* @source 0x801D089C @kind data */

/* @source 0x801CF998
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 3, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801D089C with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call, and the slot after the index byte and the slot
 * after the table load stay nops. Its only in-image reference is the word
 * 0x801CF998 at 0x801D0894, entry 15 of that run, which is the base entry of
 * the byte-2 dispatcher at 0x801CF954.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3Table089cPlcharPlp05600_801CF998, D_801D089C);
