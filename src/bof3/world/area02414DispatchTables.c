#include "bof3/emi/dispatchers.h"

extern void (*D_801F4200[])(void); /* @source 0x801F4200 @kind data */

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F4200.
 * @source 0x801F2C14
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchScratchMode, D_801F4200);
