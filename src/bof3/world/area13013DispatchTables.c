#include "bof3/emi/dispatchers.h"

extern void (*D_801F3ED8[])(void);
extern void (*D_801F3EE4[])(void);

/* @source 0x801F2C1C
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F3ED8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable3World03, callStateTable3World03, D_801F3ED8);

/* @source 0x801F31E8
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F3EE4. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte1Table3, dispatchStateByte1Table3, D_801F3EE4);
