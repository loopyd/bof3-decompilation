#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the words from 0x801D078C through 0x801D0804 are
 * in-image code pointers, this payload's functions with the shared
 * main-executable handler 0x801B2D54 at 0x801D07E0. The leading twenty-four
 * words sit inside the 0x801D078C `textbin` row and the trailing seven in the
 * 0x801D07EC row, which the reviewed Splat layout places, so the run is only
 * referenced here. This dispatcher indexes it from 0x801D07D8, so its base is
 * entry 19 of that pointer data and the reachable entries are 0x801CFAEC
 * through 0x801CFCDC. */
extern void (*D_801D07D8[])(void); /* @source 0x801D07D8 @kind data */

/* @source 0x801CFAA8
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801D07D8 with no arguments,
 * discarding any result. The chosen entry is read from the table itself, not
 * from a frame copy. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across
 * the indirect call, and the slots after the index byte and after the table
 * load stay nops. Its only in-image reference is the word 0x801CFAA8 at
 * 0x801D07F8, entry 27 of that pointer run, so the overlay loader that steps
 * the run is its caller: it has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table07d8PlcharPlp56700_801CFAA8, D_801D07D8);
