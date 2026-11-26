#include "bof3/emi/dispatchers.h"

extern void (*D_801EFD54[])(void);
extern void (*D_801EFD5C[])(void);

/* @source 0x801EF0C4
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EFD54. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTablefd54, callStateTablefd54, D_801EFD54);

/* @source 0x801EF108
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EFD5C. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTablefd5, callStateTablefd5, D_801EFD5C);
