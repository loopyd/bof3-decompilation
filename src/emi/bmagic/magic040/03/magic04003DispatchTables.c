#include "bof3/emi/dispatchers.h"

extern void (*D_801EFC6C[])(void); /* @source 0x801EFC6C @kind data */

/* @source 0x801EEF48
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, that object's unsigned byte at offset 1 is scaled by four and the
 * selected entry of the overlay-local handler run based at 0x801EFC6C (entry 0
 * = 0x801EEF8C, entry 1 = 0x801EF248) is invoked with no arguments, so no
 * argument register is loaded. The original does not bound the index before
 * the scaled load. It reads no other state and writes none; the 0x18-byte
 * frame only keeps $ra across the indirect call, the pointer-cell load precedes
 * frame creation, and the index load, the scaled index and the table load each
 * follow a nop delay slot. The 68-byte body is instruction-for-instruction
 * identical to the work-byte-1 dispatcher of emi/bmagic/magic001/00
 * (0x801EFAB8), differing only in the %lo table-address immediate of the lw.
 * No word in this payload addresses 0x801EEF48, so the caller is outside the
 * overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic04003_801EEF48, D_801EFC6C);
