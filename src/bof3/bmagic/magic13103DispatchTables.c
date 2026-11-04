#include "bof3/emi/dispatchers.h"

extern void (*dispatchHandlerTable801F1BC4[])(void); /* @source 0x801F1BC4 @kind data */

/* @source 0x801EF668
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 is scaled by four and selects an entry of the
 * overlay-local code-pointer table at 0x801F1BC4 (the entry is read back with
 * lw from that table), and the selected entry is invoked with no arguments. It
 * reads no state other than that byte and writes none; the 0x18-byte frame only
 * keeps $ra across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic13103_801EF668, dispatchHandlerTable801F1BC4);
