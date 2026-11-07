#include "bof3/emi/dispatchers.h"

extern void (*D_801F47E8[])(void);

/* @source 0x801F4DCC
 * @kind data
 */
extern void (*D_801F4DCC[])(void);

/* @source 0x801F2CE8
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F47E8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table47e8World03Area14113_801F2CE8, D_801F47E8);

/* @source 0x801F3838
 * @behavior Overlay step dispatch: reads the state byte at +4 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4DCC, whose first two
 *           entries are the routines at 0x801F387C and 0x801F390C. Takes
 *           no arguments and returns nothing, and its 0x18-byte frame
 *           only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchStateByte4Table4dccWorld03Area14113_801F3838, D_801F4DCC);
