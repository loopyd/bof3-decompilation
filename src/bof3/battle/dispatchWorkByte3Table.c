#include "bof3/battle/battle03_internal.h"
#include "bof3/emi/dispatchers.h"

/* @source 0x801DFC70
 * @behavior loads the non-volatile scratchpad pointer cell at 0x44, uses byte 3
 * of its pointed-to local work as an index, and calls that handler-table entry.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3Table, D_801EB1D4);
