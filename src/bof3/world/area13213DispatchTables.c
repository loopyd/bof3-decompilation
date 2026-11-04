#include "bof3/emi/dispatchers.h"

extern void (*D_801F3DD8[])(void);
extern void (*D_801F3DE4[])(void);
extern void (*D_801F3DEC[])(void);
extern void (*D_801F3DF8[])(void);
extern void (*D_801F3E00[])(void);

/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F3DD8.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table3dd8WorldWorld03Area13213_801F2C04, D_801F3DD8);

/* @source 0x801F2DB0
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F3DE4.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table3de4WorldWorld03Area13213_801F2DB0, D_801F3DE4);

/* @source 0x801F2DF4
 * @behavior Overlay step dispatch: reads the state byte at +2 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F3DEC.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table3decWorldWorld03Area13213_801F2DF4, D_801F3DEC);

/* @source 0x801F2F54
 * @behavior Overlay step dispatch: reads the state byte at +2 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F3DF8.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table3df8WorldWorld03Area13213_801F2F54, D_801F3DF8);

/* @source 0x801F307C
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F3E00.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table3e00WorldWorld03Area13213_801F307C, D_801F3E00);
