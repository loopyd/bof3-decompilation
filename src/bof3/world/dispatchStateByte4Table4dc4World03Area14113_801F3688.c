#include "bof3/emi/dispatchers.h"

extern void (*D_801F4DC4[])(void);

/* @source 0x801F3688
 * @behavior Overlay step dispatch: reads the state byte at +4 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F4DC4, whose four entries are
 *           the routines at 0x801F36CC, 0x801F3740, 0x801F387C and
 *           0x801F390C. Takes no arguments and returns nothing, and its
 *           0x18-byte frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchStateByte4Table4dc4World03Area14113_801F3688, D_801F4DC4);
