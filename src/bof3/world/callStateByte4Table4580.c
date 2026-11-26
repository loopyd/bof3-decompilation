#include "bof3/emi/dispatchers.h"

/* @source 0x801F4580
 * @kind data
 */
extern void (*D_801F4580[])(void);

/* @source 0x801F2EFC
 * @behavior Overlay step dispatch: reads the state byte at +4 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4580, whose four words are
 *           the routines at 0x801F2F40, 0x801F2FC4, 0x801F3068 and
 *           0x801F3108 in that order, the four step handlers of this
 *           overlay's run. Takes no arguments and returns nothing, and its
 *           0x18-byte frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, callStateByte4Table4580, D_801F4580);
