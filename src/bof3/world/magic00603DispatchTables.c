#include "bof3/bof3.h"

extern void (*D_801F0A24[])(void);

/* @source 0x801EEE04
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F0A24. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Table0a24BmagicMagic00603_801EEE04, D_801F0A24);
