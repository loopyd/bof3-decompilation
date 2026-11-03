#include "bof3/bof3.h"

extern void (*D_801F47E8[])(void);

/* @source 0x801F2CE8
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F47E8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_TWO(dispatchStateByte2Table47e8World03Area14113_801F2CE8, D_801F47E8);
