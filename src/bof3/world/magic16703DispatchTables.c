#include "bof3/bof3.h"

extern void (*D_801F08B0[])(void);

/* @source 0x801EF2D8
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F08B0. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Table08b0BmagicMagic16703_801EF2D8, D_801F08B0);
