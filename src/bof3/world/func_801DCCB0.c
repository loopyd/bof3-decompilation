#include "bof3/world/area03004_internal.h"
#include "bof3/emi/dispatchers.h"

/* @behavior dispatches the byte at offset 4 of the scratch work-record
 * cursor (0x1F800044) through the local handler table at 0x801E22F0.
 * @source 0x801DCCB0
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, func_801DCCB0, D_801E22F0);
