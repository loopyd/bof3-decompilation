#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFEDC: twenty-three code pointers, entry 0
 * = 0x801CE448 through entry 22 = 0x801CF42C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 7 and 13. The
 * target's Splat `textbin` row of the same name places entries 0 through 15 and
 * the following row carries the rest of the run, so the run is only referenced
 * here, at its first entry. */
extern void (*codePointerTablefedc[])(void); /* @source 0x801CFEDC @kind data */

/* @source 0x801CE404
 * @behavior Overlay step entry on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, before the
 * frame is created, takes that record's unsigned byte at offset 2, scales it by
 * four and calls the matching entry of this overlay's twenty-three-word handler
 * run based at 0x801CFEDC with no arguments, discarding any result. The original
 * does not bound the index before the scaled table load. It reads no other state
 * and writes none; the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call, and the slot after the index byte and the slot after the table load stay
 * nops. No jal of this payload targets the body. Its address appears once in the
 * payload, as the word 0x801CE404 at 0x801CFF20, entry 17 of that same run, so the
 * overlay loader enters the body from outside the payload while the run also
 * dispatches to it; the sibling halfword dispatcher at 0x801CFE54 indexes the run
 * from that entry, so it reaches this body as its entry 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTablefedc, callWorkTablefedc, codePointerTablefedc);
