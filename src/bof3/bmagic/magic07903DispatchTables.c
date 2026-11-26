#include "bof3/emi/dispatchers.h"

extern void (*D_801F02A4[])(void);

/* @source 0x801EED98
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F02A4. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable02, callStateTable02, D_801F02A4);
