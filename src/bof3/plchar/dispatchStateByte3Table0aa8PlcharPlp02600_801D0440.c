#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0A54: thirty code pointers, the words
 * 0x801D0A54 through 0x801D0AC8, holding this payload's functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2 and 8. The Splat
 * `textbin` row of the same name places the words, so the run is only
 * referenced here. This dispatcher indexes it from 0x801D0AA8, so its base is
 * entry 21 and the reachable entries are 0x801D0484 through 0x801CFB50. Its own
 * address is entry 15 of the same run, the word 0x801D0440 at 0x801D0A90, and
 * that word is its only in-image reference, so the overlay loader that steps
 * the run is what calls it. */
extern void (*D_801D0AA8[])(void); /* @source 0x801D0AA8 @kind data */

/* @source 0x801D0440
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 3, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801D0AA8 with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call, and the slots after the index byte and after the
 * table load stay nops. It has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchStateByte3Table0aa8PlcharPlp02600_801D0440, D_801D0AA8);
