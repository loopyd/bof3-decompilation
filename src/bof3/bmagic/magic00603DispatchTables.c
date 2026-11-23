#include "bof3/emi/dispatchers.h"

extern void (*workByte1HandlerTable[])(void); /* @source 0x801F0A24 @kind table */

/* @source 0x801EEE04
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F0A24. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateTable0Bmagic, workByte1HandlerTable);
