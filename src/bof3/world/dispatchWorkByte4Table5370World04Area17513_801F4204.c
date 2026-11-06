#include "bof3/emi/dispatchers.h"

extern void (*D_801F5370[])(void);

/* @source 0x801F4204
 * @behavior Overlay step dispatch: reads the work byte at offset 0x04 of the
 * work record published at the scratchpad pointer cell 0x1F800044 and calls
 * the handler pointer at that unchecked byte index of the local handler table
 * based at 0x801F5370, whose first two entries are the routines at 0x801F4248
 * and 0x801F426C. Takes no arguments and returns nothing, and its 0x18-byte
 * frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchWorkByte4Table5370World04Area17513_801F4204, D_801F5370);
