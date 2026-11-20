#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFEDC: twenty-three code pointers, entry 0
 * = 0x801CE448 through entry 22 = 0x801CF42C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 7 and 13. The
 * target's Splat `textbin` rows `T_801CFEDC` (entries 0 through 15) and
 * `T_801CFF1C` (entries 16 through 22) carry the run, so it is only referenced
 * here. This dispatcher indexes the run from 0x801CFEF0, so its base is entry 5
 * and the entries it can reach run from 0x801CEC40 through 0x801CF42C. */
extern void (*D_801CFEF0[])(void); /* @source 0x801CFEF0 @kind data */

/* @source 0x801CEBFC
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, before the
 * frame is created, takes that record's unsigned byte at offset 2, scales it by
 * four without bounding the index and calls the matching entry of this overlay's
 * handler run based at 0x801CFEF0 with no arguments, discarding any result. It
 * reads no other state and writes none; the 0x18-byte frame only keeps $ra at
 * 0x10 across the indirect call, and the slot after the index byte and the slot
 * after the table load stay nops. No jal of this payload targets the body. Its
 * address appears once, as the word 0x801CEBFC at 0x801CFF24, entry 18 of the run
 * at 0x801CFEDC, so the overlay loader enters the body from outside the payload
 * while the run also dispatches to it: the sibling byte-2 dispatcher at
 * 0x801CE610, based at 0x801CFEE8, reaches this body at its index 15, and this
 * body's own base 0x801CFEF0 reaches it again at its own index 13.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tablefef0PlcharPlp25700_801CEBFC, D_801CFEF0);
