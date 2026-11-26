#include "bof3/emi/dispatchers.h"

extern void (*D_801F3420[])(void);

/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F3420. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable3420World04, callStateTable3420World04, D_801F3420);
