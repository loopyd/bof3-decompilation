#include "bof3/emi/dispatchers.h"

extern void (*D_801F05D8[])(void);
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
EMI_DEFINE_DISPATCHER(callStateTable05, callStateTable05, D_801F05D8);

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
EMI_DEFINE_DISPATCHER(callStateByte1Table05, callStateByte1Table05, D_801F05F8);
