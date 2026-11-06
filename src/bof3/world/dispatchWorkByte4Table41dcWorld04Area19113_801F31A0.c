#include "bof3/emi/dispatchers.h"

extern void (*D_801F41DC[])(void); /* @source 0x801F41DC @kind data */

/* @source 0x801F31A0
 * @behavior Overlay step dispatch: reads the work byte at offset 0x04 of the
 * work record published at the scratchpad pointer cell 0x1F800044 and calls
 * the handler pointer at that unchecked byte index of the local handler table
 * based at 0x801F41DC, whose only in-image entries are the routines at
 * 0x801F31E4, 0x801F3278 and 0x801F3320. Takes no arguments and returns
 * nothing, and its 0x18-byte frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchWorkByte4Table41dcWorld04Area19113_801F31A0, D_801F41DC);
