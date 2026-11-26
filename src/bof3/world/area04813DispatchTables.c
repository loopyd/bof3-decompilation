#include "bof3/emi/dispatchers.h"

extern void (*D_801F4D68[])(void);
extern void (*D_801F4D80[])(void);
extern void (*D_801F4E54[])(void);

/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4D68. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchByte1Table4d68, dispatchByte1Table4d68, D_801F4D68);

/* @source 0x801F37B0
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4D80. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchByte2Table4d80, dispatchByte2Table4d80, D_801F4D80);

/* @source 0x801F3D60
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4E54. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchByte2Table4e54, dispatchByte2Table4e54, D_801F4E54);
