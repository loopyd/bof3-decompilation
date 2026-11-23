#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0630: thirty code pointers, entry 0 =
 * 0x801CE448 through entry 29 = 0x801CF72C, holding this payload's functions plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 8. The Splat
 * `textbin` row of the same name places the words, so the run is only referenced
 * here. This dispatcher indexes it from 0x801D0648, so its base is entry 6 and
 * the reachable entries are 0x801CF170 through 0x801CF72C. */
extern void (*D_801D0648[])(void); /* @source 0x801D0648 @kind data */

/* @source 0x801CF12C
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801D0648 with no arguments,
 * discarding any result. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the slot after the index byte and the slot after the table
 * load stay nops. Its only in-image reference is the word 0x801CF12C at
 * 0x801D0694, entry 25 of the thirty-entry code-pointer run at 0x801D0630: the
 * body's own base is entry 6, so the work byte reading 19 re-enters this
 * dispatcher, and the other byte-2 dispatchers of this payload based at or below
 * that entry reach it as well.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTable0648Plchar, D_801D0648);
