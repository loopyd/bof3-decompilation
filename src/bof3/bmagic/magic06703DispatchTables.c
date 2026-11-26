#include "bof3/emi/dispatchers.h"

extern void (*D_801F04B8[])(void);
extern void (*D_801F04E8[])(void);

/* @source 0x801EF2D0
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F04B8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateByte1Table04, callStateByte1Table04, D_801F04B8);

/* @source 0x801F02A0
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F04E8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateByte2Table04, callStateByte2Table04, D_801F04E8);
