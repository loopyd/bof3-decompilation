#include "bof3/emi/dispatchers.h"

extern void (*D_801F6778[])(void);

/* @source 0x801F471C
 * @behavior Overlay step dispatch: reads the work byte at offset 0x04 of the
 * work record published at the scratchpad pointer cell 0x1F800044 and calls
 * the handler pointer at that unchecked byte index of the local handler table
 * based at 0x801F6778, whose entries are the routines at 0x801F4760 and
 * 0x801F47F0. Takes no arguments and returns nothing, and its 0x18-byte frame
 * only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchWorkByte4Table6778World04Area17213_801F471C, D_801F6778);
