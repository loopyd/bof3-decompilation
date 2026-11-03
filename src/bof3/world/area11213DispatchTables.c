#include "bof3/bof3.h"

extern void (*D_801F41F8[])(void);
extern void (*D_801F493C[])(void);

/* @source 0x801F2C18
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F41F8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_TWO(dispatchStateByte2Table41f8World02Area11213_801F2C18, D_801F41F8);

/* @source 0x801F3E14
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F493C. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Table493cWorld02Area11213_801F3E14, D_801F493C);
