#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFEDC: twenty-three code pointers, entry 0
 * = 0x801CE448 through entry 22 = 0x801CF42C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 7 and 13. The
 * target's Splat `textbin` rows `codePointerTablefedc` (entries 0 through 15) and
 * `T_801CFF1C` (entries 16 through 22) carry the run, so it is only referenced
 * here. This dispatcher indexes the run from 0x801CFEFC, so its base is entry 8
 * and the entries it can reach run from 0x801CEE4C through 0x801CF42C. */
extern void (*D_801CFEFC[])(void); /* @source 0x801CFEFC @kind data */

/* @source 0x801CEE08
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, before the
 * frame is created, takes that record's unsigned byte at offset 2, scales it by
 * four without bounding the index and calls the matching entry of this overlay's
 * handler run based at 0x801CFEFC with no arguments, discarding any result. It
 * reads no other state and writes none; the 0x18-byte frame only keeps $ra at
 * 0x10 across the indirect call, and the slot after the index byte and the slot
 * after the table load stay nops. No jal of this payload targets the body. Its
 * address appears once, as the word 0x801CEE08 at 0x801CFF30, entry 21 of the run
 * at 0x801CFEDC, so the overlay loader enters the body from outside the payload
 * while this body's own base 0x801CFEFC reaches it again at index 13, and each of
 * this overlay's other byte-2 dispatchers reaches it from its own base: 0x801CE404
 * at 21, 0x801CE610 at 18, 0x801CEBFC at 16, 0x801CF1F8 at 10 and 0x801CF42C at 7.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTablefefc, D_801CFEFC);
