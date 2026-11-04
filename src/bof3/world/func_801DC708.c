#include "bof3/world/area03004_internal.h"
#include "bof3/emi/dispatchers.h"

/* @behavior dispatches the byte at offset 3 of the scratch work-record
 * cursor (0x1F800044) through the local handler table at 0x801E22C8.
 * @source 0x801DC708
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, func_801DC708, D_801E22C8);
