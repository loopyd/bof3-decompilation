#include "bof3/emi/dispatchers.h"

extern void (*D_801F567C[])(void);

/* @source 0x801F3E3C
 * @behavior Overlay step dispatch: reads the state byte at +4 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F567C. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchStateByte4Table567cWorld01Area04113_801F3E3C, D_801F567C);
