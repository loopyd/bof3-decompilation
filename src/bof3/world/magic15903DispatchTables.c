#include "bof3/bof3.h"

extern void (*D_801EF420[])(void);

/* @source 0x801EEDE4
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EF420. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Tablef420BmagicMagic15903_801EEDE4, D_801EF420);
