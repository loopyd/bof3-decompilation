#include "bof3/emi/dispatchers.h"

extern void (*D_801F67D8[])(void);
extern void (*D_801F67E0[])(void);
extern void (*D_801F6870[])(void);
extern void (*D_801F6894[])(void);
extern void (*D_801F689C[])(void);
extern void (*D_801F68A4[])(void);

void stateMaskChargeDecay(void);

/* @source 0x801F3434
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F67D8.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchByte1Table67d8, D_801F67D8);

/* @source 0x801F34E0
 * @behavior Overlay step dispatch: reads the state byte at +2 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F67E0.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchState02, D_801F67E0);

/* @source 0x801F418C
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F6870.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchState01, D_801F6870);

/* @source 0x801F45E0
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F6894.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchByte1Table6894, D_801F6894);

/* @source 0x801F4B00
 * @behavior Overlay step dispatch: reads the state byte at +2 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F689C,
 *           then runs the shared mask-charge decay update at 0x801F5798.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER_THEN_RESET(
    EMI_DISPATCH_WORK_BYTE_2,
    callUpdateMaskCharge,
    D_801F689C, stateMaskChargeDecay);

/* @source 0x801F5918
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F68A4.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchByte1Table68a4, D_801F68A4);
