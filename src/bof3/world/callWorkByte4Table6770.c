#include "bof3/emi/dispatchers.h"

extern void (*D_801F6770[])(void);

/* @source 0x801F43C8
 * @behavior Overlay step dispatch: reads the work byte at offset 0x04 of the
 * work record published at the scratchpad pointer cell 0x1F800044 and calls
 * the handler pointer at that unchecked byte index of the local handler table
 * based at 0x801F6770, whose entries are the routines at 0x801F440C and
 * 0x801F44C4. Takes no arguments and returns nothing, and its 0x18-byte frame
 * only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkByte4Table6770, callWorkByte4Table6770, D_801F6770);
