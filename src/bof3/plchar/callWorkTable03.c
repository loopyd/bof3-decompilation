#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D03AC: twenty-eight code pointers,
 * 0x801D03AC through 0x801D0418, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2 and 18. The Splat
 * `textbin` row T_801D03AC places all twenty-eight words, so the run is only
 * referenced here, at its first entry. */
extern void (*T_801D03AC[])(void); /* @source 0x801D03AC @kind data */

/* @source 0x801CE404
 * @behavior Overlay step entry on work-record byte 2: loads the work-record
 *   pointer published at the scratchpad pointer cell 0x1F800044 once, before the
 *   frame is created, takes that record's unsigned byte at offset 2, scales it by
 *   four and calls the matching entry of this overlay's twenty-eight-word handler
 *   run based at 0x801D03AC with no arguments, discarding any result. The
 *   original does not bound the index before the scaled table load. It reads no
 *   other state and writes none; the 0x18-byte frame only keeps $ra at 0x10
 *   across the indirect call, and the slot after the index byte and the slot
 *   after the table load stay nops. No jal of this payload targets the body. Its
 *   address appears once in the payload, as the word 0x801CE404 at 0x801D0404,
 *   entry 22 of the same run, which the halfword dispatcher at 0x801D0324
 *   indexes from that entry; the loader that steps the run enters the body from
 *   outside the payload, so its argument roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTable03, T_801D03AC);
