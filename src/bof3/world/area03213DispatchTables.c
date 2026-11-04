#include "bof3/emi/dispatchers.h"

extern void (*D_801F3F6C[])(void); /* @source 0x801F3F6C @kind data */
extern void (*D_801F3F80[])(void); /* @source 0x801F3F80 @kind data */

/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F3F6C.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table3f6cWorld00Area03213_801F2C04, D_801F3F6C);

/* @behavior dispatches the scratch work byte at offset 1 of the scratchpad
 * cursor record through the local handler table at 0x801F3F80.
 * @source 0x801F30A8
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchScratchWorkByte1, D_801F3F80);
