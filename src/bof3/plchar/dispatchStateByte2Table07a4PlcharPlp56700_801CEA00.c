#include "bof3/emi/dispatchers.h"

/* This overlay's handler table at 0x801D07A4: code pointers belonging to this
 * overlay's own handler functions, with the shared main-executable handler
 * 0x801B2D54 in the run as well. The words sit inside the pointer data of the
 * 0x801D078C `textbin` row, which the reviewed Splat layout places, so the
 * table is only referenced here. */
extern void (*D_801D07A4[])(void); /* @source 0x801D07A4 @kind data */

/* @source 0x801CEA00
 * @behavior Overlay step dispatch: reads the work-record pointer published at
 * the scratchpad pointer cell 0x1F800044 once, takes its byte at offset 2,
 * scales that byte by four and calls the matching entry of this overlay's
 * handler table at 0x801D07A4 with no arguments, discarding any result. The
 * chosen entry is read from the table itself, not from a frame copy. It reads
 * no other state and writes none; the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table07a4PlcharPlp56700_801CEA00, D_801D07A4);
