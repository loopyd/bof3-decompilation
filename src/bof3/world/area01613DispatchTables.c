#include "bof3/emi/dispatchers.h"

extern void (*D_801F5114[])(void);
extern void (*D_801F511C[])(void);
extern void (*D_801F51AC[])(void);
extern void (*D_801F51D0[])(void);

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F5114.
 * @source 0x801F341C
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, func_801F341C, D_801F5114);

/* @behavior dispatches through the local handler table selected by scratchpad
 * state byte `0x02`.
 * @source 0x801F34C8
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchState02, D_801F511C);

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F51AC.
 * @source 0x801F4178
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchArea016ScratchMode, D_801F51AC);

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F51D0.
 * @source 0x801F45CC
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, func_801F45CC, D_801F51D0);
