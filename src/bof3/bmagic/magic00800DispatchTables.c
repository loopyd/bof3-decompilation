#include "bof3/emi/dispatchers.h"

extern void (*dispatchHandlerTable801F1BEC[])(void); /* @source 0x801F1BEC @kind data */

/* @source 0x801F0250
 * @behavior Dispatches one frame of the overlay's work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 is scaled by four and selects an entry of the
 * overlay-local handler table at 0x801F1BEC (six entries: 0x801F0294,
 * 0x801F0568, 0x801F0964, 0x801F0DA8, 0x801F1294, 0x801F1708), and that entry
 * is invoked with no arguments. It reads no state other than that byte and
 * writes none; the 0x18-byte frame only keeps $ra across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic00800_801F0250, dispatchHandlerTable801F1BEC);
