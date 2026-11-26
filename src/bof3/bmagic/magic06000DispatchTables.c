#include "bof3/emi/dispatchers.h"

extern void (*D_801F0BB0[])(void);
extern void (*D_801F0BDC[])(void);

/* @source 0x801EFF44
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F0BB0. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte1Table0, dispatchStateByte1Table0, D_801F0BB0);

/* @source 0x801F07E8
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F0BDC. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateByte2Table0, callStateByte2Table0, D_801F0BDC);
