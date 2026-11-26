#include "bof3/emi/dispatchers.h"

extern void (*D_801F05B4[])(void);
extern void (*D_801F05C8[])(void);

/* @source 0x801EEF80
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F05B4. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte1Table05, dispatchStateByte1Table05, D_801F05B4);

/* @source 0x801EFF40
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F05C8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkByte1Handler31, callWorkByte1Handler31, D_801F05C8);
