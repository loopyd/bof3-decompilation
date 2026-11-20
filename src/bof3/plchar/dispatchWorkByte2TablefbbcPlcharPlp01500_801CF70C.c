#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFB84: sixteen code pointers, entry 0 =
 * 0x801CE448 through entry 15 = 0x801CF91C. This dispatcher indexes it from
 * 0x801CFBBC, so its base is entry 14 of that run, and the only entries it
 * reaches inside the run are 0x801CF750 at step 0 and 0x801CF91C at step 1; a
 * larger index continues into the seven-pointer row T_801CFBC4 that follows.
 * Splat `textbin` rows of those two names place the words, so the run is only
 * referenced here. */
extern void (*D_801CFBBC[])(void); /* @source 0x801CFBBC @kind data */

/* @source 0x801CF70C
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, the step counter the
 *           payload step bodies increment, scales it by four and calls the
 *           matching entry of this overlay's handler run based at 0x801CFBBC with
 *           no arguments, discarding any result. That base is entry 14 of the
 *           sixteen-entry run at 0x801CFB84, so step 0 reaches 0x801CF750 and
 *           step 1 reaches 0x801CF91C; the original does not bound the index
 *           before the scaled table load. It reads no other state and writes
 *           none; the record-pointer load precedes frame creation, the 0x18-byte
 *           frame only keeps $ra at 0x10 across the indirect call, and the slots
 *           after the index byte and after the table load stay nops. Its only
 *           in-image reference is the word 0x801CF70C at 0x801CFBDC, entry 6 of
 *           the seven-entry code-pointer run at 0x801CFBC4 that the sibling
 *           dispatcher at 0x801CFAFC indexes from 0x801CFBC8 on work halfword
 *           0x2C, so that dispatcher and the overlay loader that steps the run are
 *           what enter it: no jal target in this payload and no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2TablefbbcPlcharPlp01500_801CF70C, D_801CFBBC);
