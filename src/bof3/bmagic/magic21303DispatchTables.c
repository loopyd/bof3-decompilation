#include "bof3/emi/dispatchers.h"

extern void (*D_801EF8A0[])(void);
extern void (*D_801EF8A4[])(void);
extern void (*D_801EF8B4[])(void);

/* @source 0x801EEFC0
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EF8A0. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte1Tablef8, dispatchStateByte1Tablef8, D_801EF8A0);

/* @source 0x801EF004
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EF8A4. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateByte2Handlers, callStateByte2Handlers, D_801EF8A4);

/* @source 0x801EF4E0
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EF8B4. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchByte1Tablef8Tail, dispatchByte1Tablef8Tail, D_801EF8B4);
