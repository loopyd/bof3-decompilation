#include "bof3/emi/dispatchers.h"

extern void (*dispatchHandlerTable801F04B4[])(void); /* @source 0x801F04B4 @kind data */
extern void (*D_801F04B8[])(void); /* @source 0x801F04B8 @kind data */

/* @source 0x801EEFD8
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F04B4. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table04b4BmagicMagic00100_801EEFD8, dispatchHandlerTable801F04B4);

/* @source 0x801EFAB8
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, that object's unsigned byte at offset 1 is scaled by four and the
 * selected entry of the overlay-local handler run based at 0x801F04B8 (its
 * only in-image code pointer is 0x801EFAFC at index 0) is invoked with no
 * arguments, so no argument register is loaded. It reads no other state and
 * writes none; the 0x18-byte frame only keeps $ra across the indirect call, the
 * pointer-cell load precedes frame creation, and the index load, the scaled
 * index and the table load each follow a nop delay slot. The 68-byte body is
 * instruction-for-instruction identical to the sibling dispatcher at
 * 0x801EEFD8 of this overlay and to the work-byte-1 dispatcher of
 * emi/bmagic/magic009/03 (0x801EF0DC), differing only in the %lo table-address
 * immediate of the lw.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic00100_801EFAB8, D_801F04B8);
