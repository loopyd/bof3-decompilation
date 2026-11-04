#include "bof3/emi/dispatchers.h"

extern void (*D_801EFA9C[])(void);

/* @source 0x801EEFF8
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EFA9C. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Tablefa9cBmagicMagic11103_801EEFF8, D_801EFA9C);
