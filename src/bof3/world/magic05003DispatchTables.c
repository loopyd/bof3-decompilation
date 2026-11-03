#include "bof3/bof3.h"

extern void (*D_801EFA9C[])(void);

/* @source 0x801EEF08
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EFA9C. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Tablefa9cBmagicMagic05003_801EEF08, D_801EFA9C);
