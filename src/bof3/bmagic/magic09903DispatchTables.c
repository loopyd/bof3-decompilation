#include "bof3/emi/dispatchers.h"

extern void (*workByte1HandlerTable[])(void); /* @source 0x801F0994 @kind table */

/* Encoded addresses read by the work-byte-2 dispatcher at 0x801EF4C4.
 * Selector bounds and callback signatures remain unresolved.
 * @source 0x801F09B0 @kind table */
extern u32 workByte2HandlerTableB[];

/* @source 0x801EF16C
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F0994. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateTable0994Bmagic, workByte1HandlerTable);
