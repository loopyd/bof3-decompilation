#include "bof3/emi/dispatchers.h"

extern void (*D_801F33EC[])(void); /* @source 0x801F33EC @kind data */

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F33EC.
 * @source 0x801F2C04
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchArea026ScratchMode, D_801F33EC);
