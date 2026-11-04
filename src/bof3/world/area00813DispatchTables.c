#include "bof3/emi/dispatchers.h"

extern void (*modeHandlerTable[])(void); /* @source 0x801F4688 @kind data */
extern void (*areaStateModeHandlerTable[])(void); /* @source 0x801F46B0 @kind data */
extern void (*D_801F46EC[])(void); /* @source 0x801F46EC @kind data */

/* @behavior dispatches the current scratchpad mode through the local handler
 * table.
 * @source 0x801F2C18
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchMode, modeHandlerTable);

/* @behavior dispatches the current area-state mode byte through the local
 * handler table at 0x801F46B0.
 * @source 0x801F3244
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, func_801F3244, areaStateModeHandlerTable);

/* @behavior dispatches the current area-state mode byte through the local
 * handler table at 0x801F46EC.
 * @source 0x801F3A64
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, func_801F3A64, D_801F46EC);
