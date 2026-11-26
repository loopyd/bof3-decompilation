#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0630: thirty code pointers, entry 0 =
 * 0x801CE448 through entry 29 = 0x801CF72C, holding this payload's functions plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 8. The Splat
 * `textbin` row of the same name places the words, so the run is only referenced
 * here. This dispatcher indexes it from the run base, so entry 0 is reachable. */
extern void (*codePointerTable0630[])(void); /* @source 0x801D0630 @kind data */

/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's thirty-entry handler run based at 0x801D0630 with no
 * arguments, discarding any result. The original does not bound the index before
 * the scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across
 * the indirect call, and the slot after the index byte and the slot after the table
 * load stay nops. No word and no jump of this payload names the body and no table
 * word holds it, so the shared main executable runs it as this overlay's step
 * entry and the argument roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTable0630, callWorkTable0630, codePointerTable0630);
