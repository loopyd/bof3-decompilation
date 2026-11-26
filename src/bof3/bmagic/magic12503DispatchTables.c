#include "bof3/emi/dispatchers.h"

extern void (*workByte1HandlerTable[])(void); /* @source 0x801F05D8 @kind table */
extern void (*D_801F05F8[])(void);

/* @source 0x801EEEC0
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F05D8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateTable05, workByte1HandlerTable);

/* @source 0x801EFB48
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F05F8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateByte1Table05, D_801F05F8);
