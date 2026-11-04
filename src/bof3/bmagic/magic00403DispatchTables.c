#include "bof3/emi/dispatchers.h"

extern void (*dispatchHandlerTable801EFE58[])(void); /* @source 0x801EFE58 @kind data */

/* @source 0x801EF8CC
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 is scaled by four and selects an entry of the
 * five-entry overlay-local code-pointer table at 0x801EFE58 (0x801EF910,
 * 0x801EFAF4, 0x801EFBA8, 0x801EFBEC, 0x801EFD84), and the selected entry is
 * invoked with no arguments. It reads no other state and writes none; the
 * 0x18-byte frame only keeps $ra across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic00403_801EF8CC, dispatchHandlerTable801EFE58);
