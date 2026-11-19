#include "bof3/emi/dispatchers.h"

extern void (*D_801F5C94[])(void);

/* @source 0x801F3DCC
 * @behavior Overlay per-frame work dispatcher: reads the work record's state
 *           byte at +4 through the scratchpad pointer cell 0x1F800044, scales
 *           it by four and calls the handler pointer at that index of the
 *           overlay handler table based at 0x801F5C94, whose two entries are
 *           the routines at 0x801F3E10 and 0x801F3F18. Takes no arguments and
 *           returns nothing; the 0x18-byte frame only keeps $ra across the
 *           indirect call, and the pointer-cell load precedes frame creation.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchStateByte4Table5c94World03Area14514_801F3DCC, D_801F5C94);
