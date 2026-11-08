#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF5EC: sixteen code pointers, 0x801CF5EC
 * through 0x801CF628, holding this payload's own functions plus the shared
 * main-executable handler 0x801B2D54 at entries 2 and 15. The Splat `textbin`
 * row at 0x801CF5EC places the words, so the run is only referenced here. This
 * dispatcher indexes it from 0x801CF620, its fourteenth entry, so the base is
 * 0x801CF620 and the three reachable entries are 0x801CF36C, 0x801CF438 and
 * 0x801B2D54. That base is three entries past the one this overlay's byte-3
 * dispatcher at 0x801CEF10 uses. */
extern void (*D_801CF620[])(void); /* @source 0x801CF620 @kind data */

/* @source 0x801CF328
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CF620 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CF328 at 0x801CF630, word 1 of the six-word code-pointer run at
 *           0x801CF62C that this target's Splat row still declares as code, so
 *           it has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tablef620PlcharPlp34900_801CF328, D_801CF620);
