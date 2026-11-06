#include "bof3/emi/dispatchers.h"

extern void (*D_801F3FF4[])(void);
extern void (*D_801F4004[])(void);

/* @source 0x801F4DCC
 * @kind data
 */
extern void (*D_801F4DCC[])(void);

/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F3FF4. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table3ff4World01Area06813_801F2C04, D_801F3FF4);

/* @source 0x801F2DA4
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4004. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table4004World01Area06813_801F2DA4, D_801F4004);

/* @source 0x801F3B18
 * @behavior Overlay step dispatch: reads the state byte at +4 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4DCC. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchStateByte4Table4dccWorld01Area06813_801F3B18, D_801F4DCC);
