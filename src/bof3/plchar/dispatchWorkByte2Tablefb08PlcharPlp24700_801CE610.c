#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAFC: twenty code pointers filling the
 * payload's trailing read-only block, holding this payload's functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 6 and 10. The target's
 * Splat `textbin` row places the whole run, so it is only referenced here. This
 * dispatcher indexes it from 0x801CFB08, its fourth entry, so its first reachable
 * handler is 0x801CE654 and the seventeen reachable entries run from there to
 * 0x801CF04C. That starting entry sits five entries before the table base of the
 * overlay's other byte-2 dispatcher at 0x801CEE18, which is 0x801CFB1C. */
extern void (*D_801CFB08[])(void); /* @source 0x801CFB08 @kind data */

/* @source 0x801CE610
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801CFB08 with no arguments,
 * discarding any result. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the slot after the index byte and the slot after the table
 * load stay nops. Its only in-image reference is the word 0x801CE610 at
 * 0x801CFB40, the seventeenth word of the run based at 0x801CFAFC, which this
 * dispatcher's own base reaches at byte-2 index 14, so the overlay loader that
 * steps that run is what calls it. The 68-byte body is the same 17 instructions
 * as this overlay's byte-2 dispatchers at 0x801CEE18 and 0x801CF04C word for
 * word except the `lw` offset that names this table base.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tablefb08PlcharPlp24700_801CE610, D_801CFB08);
