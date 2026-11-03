#include "bof3/bof3.h"

extern void (*D_801F47C4[])(void);
extern void (*D_801F47DC[])(void);
extern void (*D_801F4AC4[])(void);

/* @source 0x801F2C34
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F47C4. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_TWO(dispatchStateByte2Table47c4World03Area14013_801F2C34, D_801F47C4);

/* @source 0x801F32DC
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F47DC. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_TWO(dispatchStateByte2Table47dcWorld03Area14013_801F32DC, D_801F47DC);

/* @source 0x801F46A0
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4AC4. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Table4ac4World03Area14013_801F46A0, D_801F4AC4);
