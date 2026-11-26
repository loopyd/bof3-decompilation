#include "bof3/emi/dispatchers.h"

extern void (*D_801F4B38[])(void);
extern void (*D_801F4B88[])(void);
extern void (*D_801F4BA0[])(void);
extern void (*D_801F4BB8[])(void);

/* @source 0x801F2C38
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4B38. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte1Table4, dispatchStateByte1Table4, D_801F4B38);

/* @source 0x801F33E0
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4B88. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte2Table4, dispatchStateByte2Table4, D_801F4B88);

/* @source 0x801F3D6C
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4BA0. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte2Table4ba0World04Area16813_801F3D6C, dispatchStateByte2Table4ba0World04Area16813_801F3D6C, D_801F4BA0);

/* @source 0x801F44B0
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4BB8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte2Table4bb8World04Area16813_801F44B0, dispatchStateByte2Table4bb8World04Area16813_801F44B0, D_801F4BB8);
