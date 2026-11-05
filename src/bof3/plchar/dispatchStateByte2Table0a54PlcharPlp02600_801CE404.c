#include "bof3/emi/dispatchers.h"

/* This overlay's handler table at 0x801D0A54: thirty code pointers, from
 * 0x801CE448 through 0x801CFB50, with the shared main-executable handler
 * 0x801B2D54 at entries 2 and 8. The Splat `textbin` row of the same name
 * places the words, so the table is only referenced here. */
extern void (*T_801D0A54[])(void); /* @source 0x801D0A54 @kind data */

/* @source 0x801CE404
 * @behavior Overlay step dispatch: reads the work-record pointer published at
 * the scratchpad pointer cell 0x1F800044 once, takes its byte at offset 2,
 * scales that byte by four and calls the matching entry of this overlay's
 * thirty-entry handler table at 0x801D0A54 with no arguments, discarding any
 * result. The chosen entry is read from the table itself, not from a frame
 * copy. It reads no other state and writes none; the 0x18-byte frame only
 * keeps $ra at 0x10 across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table0a54PlcharPlp02600_801CE404, T_801D0A54);
