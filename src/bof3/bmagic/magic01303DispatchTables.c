#include "bof3/emi/dispatchers.h"

extern void (*dispatchHandlerTable801F03BC[])(void); /* @source 0x801F03BC @kind data */
extern void (*D_801F03D8[])(void); /* @source 0x801F03D8 @kind data */

/* @source 0x801EEFD0
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F03BC. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table03bcBmagicMagic01303_801EEFD0, dispatchHandlerTable801F03BC);

/* @source 0x801EFDB4
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its unsigned byte at offset 1 is scaled by four and selects an entry of the
 * overlay-local three-entry code-pointer run at 0x801F03D8 (entry 0 =
 * 0x801EFDF8, entry 1 = 0x801EFE80, entry 2 = 0x801EFF88), and that entry is
 * invoked with no arguments. It reads no state other than that byte and writes
 * none; the 0x18-byte frame only keeps $ra across the indirect call, the
 * pointer-cell load precedes frame creation, and the lbu and the table lw are
 * each followed by a nop. No word in this payload addresses 0x801EFDB4, so the
 * caller is outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic01303_801EFDB4, D_801F03D8);
