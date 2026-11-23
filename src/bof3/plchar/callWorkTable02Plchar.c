#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D02F4: twenty-eight code pointers, entry 0
 * = 0x801CE448 through entry 27 = 0x801D025C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 8 and 21. The
 * Splat `textbin` row of the same name places the words, so the run is only
 * referenced here. This dispatcher indexes it from the run base, so entry 0 is
 * reachable and every one of the twenty-eight words is. */
extern void (*T_801D02F4[])(void); /* @source 0x801D02F4 @kind data */

/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's twenty-eight-word handler run based at 0x801D02F4 with
 * no arguments, discarding any result. The original does not bound the index
 * before the scaled table load. It reads no other state and writes none; the
 * pointer-cell load precedes frame creation, the 0x18-byte frame only keeps $ra
 * at 0x10 across the indirect call, and the slot after the index byte and the
 * slot after the table load stay nops. No jal in this payload targets the body;
 * its address appears once in the payload, as the word 0x801CE404 at 0x801D034C,
 * entry 22 of the run, which is also the base the sibling halfword dispatcher
 * callWorkTable034Plchar indexes, so that
 * dispatcher reaches this body as its entry 0 while the overlays enter the body
 * itself from outside the payload. It takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTable02Plchar, T_801D02F4);
