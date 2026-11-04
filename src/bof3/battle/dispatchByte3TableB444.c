#include "bof3/battle/battle03_internal.h"
#include "bof3/emi/dispatchers.h"

/* @source 0x801E4760
 * @behavior loads the non-volatile scratchpad pointer cell at 0x1F800044, then invokes its byte-3-selected handler from the local 0x801EB444 dispatch table
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchByte3TableB444, D_801EB444);
