#include "bof3/emi/dispatchers.h"

extern void (*workByte1HandlerTable[])(void); /* @source 0x801F33EC @kind data */

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F33EC.
 * @source 0x801F2C04
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchScratchModeLocal, workByte1HandlerTable);
