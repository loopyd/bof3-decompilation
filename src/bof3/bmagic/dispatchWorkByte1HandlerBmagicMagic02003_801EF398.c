#include "bof3/emi/dispatchers.h"

extern void (*D_801EFEB8[])(void); /* @source 0x801EFEB8 @kind data */

/* @source 0x801EF398
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, that object's unsigned byte at offset 1 is scaled by four and the
 * selected entry of the three-entry overlay-local handler run based at
 * 0x801EFEB8 (0x801EF3DC, 0x801EF894, 0x801EFAF4) is invoked with no
 * arguments, so no argument register is loaded. The original does not bound
 * the index before the scaled load. It reads no other state and writes none;
 * the 0x18-byte frame only keeps $ra across the indirect call, and the
 * pointer-cell load precedes frame creation.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic02003_801EF398, D_801EFEB8);
