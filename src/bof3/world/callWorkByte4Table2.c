#include "bof3/emi/dispatchers.h"

extern void (*D_801F4DE4[])(void);

/* @source 0x801F3C2C
 * @behavior Overlay step dispatch through the byte-4 handler table at
 *           0x801F4DE4, whose index 0 names the phase armer at 0x801F3C70 and
 *           index 1 the phase mover at 0x801F3CB0: reads the work record
 *           published at the scratchpad pointer cell 0x1F800044, takes its
 *           byte at 0x04 as the table index, scales that byte by four and
 *           calls the resulting table entry with no arguments, reaching the
 *           handler only through the indexed pointer word. Keeps a 0x18 frame
 *           to hold $ra across that call, takes no arguments and returns
 *           nothing, so the value left in $v0 comes from that callee.
 *           The payload publishes this routine itself as entry 4 of the
 *           eight-entry step pointer run at 0x801F4D3C.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, callWorkByte4Table2, D_801F4DE4);
