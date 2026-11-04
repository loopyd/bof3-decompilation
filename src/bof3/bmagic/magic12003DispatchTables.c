#include "bof3/emi/dispatchers.h"

extern void (*D_801F0CA8[])(void);

/* @source 0x801EF010
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F0CA8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table0ca8BmagicMagic12003_801EF010, D_801F0CA8);
