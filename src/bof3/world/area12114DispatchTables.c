#include "bof3/emi/dispatchers.h"

extern void (*D_801F67D8[])(void);
extern void (*D_801F67E0[])(void);
extern void (*D_801F6870[])(void);
extern void (*D_801F6894[])(void);
extern void (*D_801F689C[])(void);
extern void (*D_801F68A4[])(void);

void updateMaskChargeAndDecayWorkByteBWorld03Area12114_801F5798(void);

/* @source 0x801F3434
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F67D8.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table67d8World03Area12114_801F3434, D_801F67D8);

/* @source 0x801F34E0
 * @behavior Overlay step dispatch: reads the state byte at +2 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F67E0.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchState02World03Area12114_801F34E0, D_801F67E0);

/* @source 0x801F418C
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F6870.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchState01World03Area12114_801F418C, D_801F6870);

/* @source 0x801F45E0
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F6894.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table6894World03Area12114_801F45E0, D_801F6894);

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
    dispatchStateByte2Table689cThenUpdateMaskChargeWorld03Area12114_801F4B00,
    D_801F689C, updateMaskChargeAndDecayWorkByteBWorld03Area12114_801F5798);

/* @source 0x801F5918
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F68A4.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table68a4World03Area12114_801F5918, D_801F68A4);
