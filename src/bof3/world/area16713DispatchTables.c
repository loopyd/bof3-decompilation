#include "bof3/emi/dispatchers.h"

extern void (*D_801F4A88[])(void);
extern void (*D_801F4AA0[])(void);
extern void (*D_801F4AB0[])(void);

/* @source 0x801F2C88
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4A88. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte2Table4, dispatchStateByte2Table4, D_801F4A88);

/* @source 0x801F35E0
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4AA0. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte2Table4aa0World04Area16713_801F35E0, dispatchStateByte2Table4aa0World04Area16713_801F35E0, D_801F4AA0);

/* @source 0x801F3764
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4AB0. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte2Table4ab0World04Area16713_801F3764, dispatchStateByte2Table4ab0World04Area16713_801F3764, D_801F4AB0);
