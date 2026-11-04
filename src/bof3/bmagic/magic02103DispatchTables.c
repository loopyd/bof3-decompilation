#include "bof3/emi/dispatchers.h"

extern void (*dispatchHandlerTable801EFE00[])(void); /* @source 0x801EFE00 @kind data */
extern void (*D_801EFE04[])(void); /* @source 0x801EFE04 @kind data */

/* @source 0x801EEFDC
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EFE00. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Tablefe00BmagicMagic02103_801EEFDC, dispatchHandlerTable801EFE00);

/* @source 0x801EF534
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its unsigned state byte at offset 1 is scaled by four and the selected entry
 * of the overlay-local handler run based at 0x801EFE04 (four in-image code
 * pointers, entry 0 = 0x801EF578 through entry 3 = 0x801EF7A0) is invoked with
 * no arguments, so no argument register is loaded. The original does not bound
 * the index before the scaled load. It reads no other state and writes none;
 * the 0x18-byte frame only keeps $ra across the indirect call, and the
 * pointer-cell load precedes frame creation. The 68-byte body is
 * instruction-for-instruction identical to the work-byte-1 dispatchers of
 * emi/bmagic/magic001/00 (0x801EFAB8), emi/bmagic/magic003/03 (0x801EFA9C),
 * emi/bmagic/magic009/03 (0x801EF0DC) and emi/bmagic/magic012/00 (0x801EEFF8),
 * differing only in the table address.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic02103_801EF534, D_801EFE04);
