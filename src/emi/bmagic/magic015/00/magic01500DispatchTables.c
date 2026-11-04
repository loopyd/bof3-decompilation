#include "bof3/emi/dispatchers.h"

extern void (*D_801F1630[])(void); /* @source 0x801F1630 @kind data */

/* @source 0x801EF9E4
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, its unsigned byte at offset 1 is scaled by four and the selected entry
 * of the four-entry overlay-local code-pointer run at 0x801F1630 (0x801EFA28,
 * 0x801EFF1C, 0x801F0334, 0x801F0C30) is invoked with no arguments, so no
 * argument register is loaded. It reads no other state and writes none; the
 * 0x18-byte frame only keeps $ra across the indirect call, the pointer-cell load
 * precedes frame creation, and the index load, the scaled index and the table
 * load each follow a nop delay slot. No word in this payload addresses
 * 0x801EF9E4, so the caller is outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic01500_801EF9E4, D_801F1630);
