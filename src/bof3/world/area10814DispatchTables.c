#include "bof3/bof3.h"

extern void (*D_801F56E8[])(void);
extern void (*D_801F56FC[])(void);
extern void (*D_801F5718[])(void);

/* @source 0x801F2C44
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F56E8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Table56e8World02Area10814_801F2C44, D_801F56E8);

/* @source 0x801F36A4
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F56FC. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Table56fcWorld02Area10814_801F36A4, D_801F56FC);

/* @source 0x801F3FF8
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F5718. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_TWO(dispatchStateByte2Table5718World02Area10814_801F3FF8, D_801F5718);
