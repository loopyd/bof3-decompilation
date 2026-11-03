#include "bof3/bof3.h"

extern void (*D_801EF7A0[])(void);

/* @source 0x801EEFF4
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EF7A0. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Tablef7a0BmagicMagic07503_801EEFF4, D_801EF7A0);
