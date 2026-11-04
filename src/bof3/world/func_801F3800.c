#include "bof3/world/area03213_internal.h"
#include "bof3/emi/dispatchers.h"

/* @behavior dispatches the scratch work byte at offset 4 of the scratchpad
 * cursor record through the local handler table at 0x801F4908.
 * @source 0x801F3800
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, func_801F3800, D_801F4908);
