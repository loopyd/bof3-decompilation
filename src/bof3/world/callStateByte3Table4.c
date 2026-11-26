#include "bof3/emi/dispatchers.h"

extern void (*D_801F4F74[])(void);

/* @source 0x801F3690
 * @behavior Overlay step dispatch: reads the state byte at +3 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4F74. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateByte3Table4, callStateByte3Table4, D_801F4F74);
