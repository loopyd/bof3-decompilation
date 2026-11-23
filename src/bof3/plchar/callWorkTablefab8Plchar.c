#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAB8: twenty-three code pointers, entry 0
 * = 0x801CE448 through entry 22 = 0x801CF868, holding this payload's own
 * functions plus the shared main-executable handler 0x801B2D54 at entries 2, 8
 * and 14. The target's Splat `textbin` row `T_801CFAB8` places entries 0
 * through 15 and the following row `T_801CFAF8` carries entries 16 through 22,
 * so the run is only referenced here, at its first entry. */
extern void (*T_801CFAB8[])(void); /* @source 0x801CFAB8 @kind data */

/* @source 0x801CE404
 * @behavior Overlay step entry on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           before the frame is created, takes that record's unsigned byte at
 *           offset 2, scales it by four and calls the matching entry of this
 *           overlay's twenty-three-word handler run based at 0x801CFAB8 with no
 *           arguments, discarding any result. The original does not bound the
 *           index before the scaled table load. It reads no other state and
 *           writes none; the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. No jal of this payload targets the body.
 *           Its address appears once in the payload, as the word 0x801CE404 at
 *           0x801CFAFC, entry 17 of the run based at 0x801CFAB8, so the overlay
 *           loader enters the body from outside the payload while the run also
 *           dispatches to it; the sibling halfword dispatcher at 0x801CFA30
 *           indexes the run from that entry and so reaches this body as its
 *           entry 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTablefab8Plchar, T_801CFAB8);
