#include "bof3/emi/dispatchers.h"

extern void (*workByte1HandlerTable[])(void); /* @source 0x801F0354 @kind table */

/* Encoded addresses read by the work-byte-2 dispatchers at 0x801EF2BC and
 * 0x801EF70C. Selector bounds and callback signatures remain unresolved. */
extern u32 workByte2HandlerTable[]; /* @source 0x801F0360 @kind table */
extern u32 tableHandlerByte2B[]; /* @source 0x801F036C @kind table */

/* @source 0x801EF278
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F0354. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable0354, callStateTable0354, workByte1HandlerTable);
