#include "bof3/emi/dispatchers.h"

extern void (*D_801F0B00[])(void); /* @source 0x801F0B00 @kind data */

/* @source 0x801EFA9C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, that object's unsigned byte at offset 1 is scaled by four and the
 * selected entry of the overlay-local handler run based at 0x801F0B00 (its
 * only in-image code pointer is 0x801EFAE0 at index 0) is invoked with no
 * arguments, so no argument register is loaded. The original does not bound
 * the index before the scaled load. It reads no other state and writes none;
 * the 0x18-byte frame only keeps $ra across the indirect call, and the
 * pointer-cell load precedes frame creation. The 68-byte body is
 * instruction-for-instruction identical to the work-byte-1 dispatchers of
 * emi/bmagic/magic001/00 (0x801EFAB8), emi/bmagic/magic009/03 (0x801EF0DC)
 * and emi/bmagic/magic012/00 (0x801EEFF8), differing only in the %lo
 * table-address immediate of the lw.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic00303_801EFA9C, D_801F0B00);
