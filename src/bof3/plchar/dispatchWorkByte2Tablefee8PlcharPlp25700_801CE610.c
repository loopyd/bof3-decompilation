#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFEDC: twenty-three code pointers, entry 0
 * = 0x801CE448 through entry 22 = 0x801CF42C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 7 and 13. The
 * target's Splat `textbin` rows `T_801CFEDC` (entries 0 through 15) and
 * `T_801CFF1C` (entries 16 through 22) carry the run, so it is only referenced
 * here. This dispatcher indexes the run from 0x801CFEE8, so its base is entry 3
 * and the entries it can reach run from 0x801CE654 through 0x801CF42C. */
extern void (*D_801CFEE8[])(void); /* @source 0x801CFEE8 @kind data */

/* @source 0x801CE610
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, before the
 * frame is created, takes that record's unsigned byte at offset 2, scales it by
 * four and calls the matching entry of this overlay's handler run based at
 * 0x801CFEE8 with no arguments, discarding any result. The original does not
 * bound the index before the scaled table load. It reads no other state and
 * writes none; the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call, and the slot after the index byte and the slot after the table load stay
 * nops. No jal of this payload targets the body. Its address appears once, as
 * the word 0x801CE610 at 0x801CFF2C, entry 20 of the run at 0x801CFEDC, so the
 * overlay loader enters the body from outside the payload while the run also
 * dispatches to it: the sibling halfword dispatcher at 0x801CFE98 indexes the run
 * from that entry and reaches this body as its entry 0, while this body's own
 * base 0x801CFEE8 reaches it again at its index 17.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tablefee8PlcharPlp25700_801CE610, D_801CFEE8);
