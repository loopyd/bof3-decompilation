#include "bof3/battle/battle03_internal.h"
#include "bof3/emi/dispatchers.h"

/* @source 0x801E3438
 * @behavior dispatches through D_801EB3F4 using byte 3 of the object addressed
 * by the non-volatile scratchpad pointer cell battleWork.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchByte3TableB3f4, D_801EB3F4);
