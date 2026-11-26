#include "bof3/emi/dispatchers.h"

/* This overlay's handler table at 0x801F3DF0: the two code pointers 0x801F2C48
 * and 0x801F2C88, which the step handler and the phase mover of the same state
 * machine occupy. The Splat `textbin` row T_801F3258 places the words, so the
 * table is only referenced here. */
extern void (*workByte4HandlerTable[])(void); /* @source 0x801F3DF0 @kind data */

/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the work-record pointer published at
 *           the scratchpad pointer cell 0x1F800044 once, takes its byte at
 *           offset 4, scales that byte by four and calls the matching entry of
 *           this overlay's two-entry handler table at 0x801F3DF0 with no
 *           arguments, discarding any result. The chosen entry is read from the
 *           table itself, not from a frame copy. It reads no other state and
 *           writes none; the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkByte4Table, callWorkByte4Table, workByte4HandlerTable);
