#include "bof3/emi/dispatchers.h"

extern void (*D_801F041C[])(void);
extern void (*D_801F0420[])(void);
extern void dispatchWorkByte2HandlerThenDrawBetweenGraphTypeDrawModesAroundHelpersWhenWorkBytes0And2SetBmagicMagic16903_801EF8A4(void); /* @source 0x801EF8A4 @kind function */

/* @source 0x801EF210
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F041C. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateTable041Bmagic, D_801F041C);

/* @source 0x801EF860
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F0420. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateTable0420Bmagic, D_801F0420);
