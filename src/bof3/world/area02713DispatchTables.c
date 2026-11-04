#include "bof3/emi/dispatchers.h"

extern void (*handlerTable[])(void); /* @source 0x801F3AB4 @kind data */
extern void (*D_801F3ABC[])(void); /* @source 0x801F3ABC @kind data */

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F3AB4.
 * @source 0x801F2C30
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchScratchMode1, handlerTable);

/* @behavior dispatches the scratch mode byte at offset 2 through the local
 * handler table at 0x801F3ABC.
 * @source 0x801F3120
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchScratchMode2, D_801F3ABC);
