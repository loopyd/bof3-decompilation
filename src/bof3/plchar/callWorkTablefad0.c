#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAB8: twenty-three code pointers, 0x801CFAB8
 * through 0x801CFB10, holding this payload's own functions plus the shared
 * main-executable handler 0x801B2D54 at entries 2, 8 and 14. The target's Splat
 * `textbin` rows `T_801CFAB8` and `workByte2RunTail` place entries 0 through 15 and 16
 * through 22, so the run is only referenced here, at the interior word this
 * dispatcher indexes. This dispatcher's base 0x801CFAD0 is entry 6 of that run, so
 * the entries it holds from there are 0x801CEA44, 0x801CEB38, 0x801B2D54,
 * 0x801CEC78, 0x801CEFC4, 0x801CF160, 0x801CF6A0, 0x801CF76C, 0x801B2D54,
 * 0x801CF8AC, 0x801CF9C4, 0x801CE404, 0x801CEA00, 0x801CF65C, 0x801CE610,
 * 0x801CEC34 and 0x801CF868. */
extern void (*D_801CFAD0[])(void); /* @source 0x801CFAD0 @kind data */

/* @source 0x801CEA00
 * @behavior Overlay step dispatcher on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           before the frame is created, takes that record's unsigned byte at
 *           offset 2, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CFAD0 with no arguments,
 *           discarding any result. The original does not bound the index before
 *           the scaled table load. It reads no other state and writes none; the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slot after the index byte and the slot after the table load stay
 *           nops. Its own address appears once in the payload, as the word
 *           0x801CEA00 at 0x801CFB00, entry 18 of the run based at 0x801CFAB8, so
 *           the overlay loader enters the body from outside the payload while the
 *           run also dispatches to it; no jal of the payload names the span.
 *           Base 0x801CFAD0 is entry 6 of that same run.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTablefad0, D_801CFAD0);
