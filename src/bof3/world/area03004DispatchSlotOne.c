#include "bof3/world/area03004_internal.h"
#include "bof3/emi/dispatchers.h"

extern void (*D_801E1CD4[])(void); /* @source 0x801E1CD4 @kind data */

/* @behavior dispatches the scratch cursor mode byte at offset 1 through the
 * second local handler table at 0x801E1D20.
 * @source 0x801D20C8
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, func_801D20C8, D_801E1D20);

/* @behavior dispatches the scratch cursor mode byte at offset 1 through the
 * third local handler table at 0x801E1D78.
 * @source 0x801D2978
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, func_801D2978, D_801E1D78);

/* @behavior dispatches the scratch cursor mode byte at offset 1 through the
 * local handler table at 0x801E1D80.
 * @source 0x801D2BF0
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, func_801D2BF0, D_801E1D80);

/* @behavior dispatches the scratch cursor mode byte at offset 1 through the
 * local handler table at 0x801E1D8C.
 * @source 0x801D3358
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, func_801D3358, D_801E1D8C);

/* @behavior dispatches the scratch cursor mode byte at offset 1 through the
 * local handler table at 0x801E1F78.
 * @source 0x801D42A0
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, func_801D42A0, D_801E1F78);

/* @behavior dispatches the scratch cursor mode byte at offset 1 through the
 * local handler table at 0x801E1D10.
 * @source 0x801D1B10
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, func_801D1B10, D_801E1D10);

/* @behavior dispatches the scratch cursor mode byte at offset 2 through the
 * local handler table at 0x801E21E0.
 * @source 0x801D9B14
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, func_801D9B14, D_801E21E0);

/* @behavior dispatches the scratch cursor mode byte at offset 2 through the
 * local handler table at 0x801E2340.
 * @source 0x801DFFA8
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, func_801DFFA8, D_801E2340);

/* @behavior dispatches the scratch cursor mode byte at offset 2 through the
 * local handler table at 0x801E2348.
 * @source 0x801E00E0
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, func_801E00E0, D_801E2348);


/* @source 0x801D0C98
 * @behavior Overlay step dispatch embedded in the leading data block: reads the
 *           state byte at +1 of the work record published at the scratchpad pointer
 *           cell 0x1F800044 and calls the handler pointer at that index of the local
 *           handler table based at 0x801E1CD4. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table1cd4World00Area03004_801D0C98, D_801E1CD4);
