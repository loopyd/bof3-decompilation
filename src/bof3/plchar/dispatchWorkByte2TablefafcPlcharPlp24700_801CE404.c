#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAFC: twenty code pointers filling the
 * payload's trailing read-only block, holding this payload's functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 6 and 10. The target's
 * Splat `textbin` row places the whole run, so it is only referenced here. This
 * dispatcher indexes it from its first entry, so its base is the run base itself
 * and all twenty entries 0x801CE448 through 0x801CF04C are reachable. The
 * overlay's other byte-2 dispatchers at 0x801CEE18 and 0x801CF04C start eight and
 * eleven entries further in along the same run. */
extern void (*T_801CFAFC[])(void); /* @source 0x801CFAFC @kind data */

/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801CFAFC with no arguments,
 * discarding any result. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the slot after the index byte and the slot after the table
 * load stay nops. Its only in-image reference is the word 0x801CE404 at
 * 0x801CFB34, entry 14 of the run at 0x801CFAFC, which this dispatcher's own base
 * reaches at byte-2 index 14, so the overlay loader that steps that run is what
 * calls it. The 68-byte body is the same 17 instructions as this overlay's byte-2
 * dispatchers at 0x801CEE18 and 0x801CF04C word for word except the `lw` offset
 * that names this table base.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2TablefafcPlcharPlp24700_801CE404, T_801CFAFC);
