#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFAFC: twenty code pointers filling the
 * payload's trailing read-only block, holding this payload's functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 7 and 10. The target's
 * Splat `textbin` row places the whole run, so it is only referenced here. This
 * dispatcher indexes it from 0x801CFB10, its fifth entry, so its fifteen
 * reachable entries run from 0x801CEC40 through 0x801CF04C. The overlay's four
 * other byte-2 dispatchers index the same run from 0x801CFAFC, 0x801CFB08,
 * 0x801CFB1C and 0x801CFB28, so this base sits two entries past the second of
 * them. */
extern void (*D_801CFB10[])(void); /* @source 0x801CFB10 @kind data */

/* @source 0x801CEBFC
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801CFB10 with no arguments,
 * discarding any result. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the slot after the index byte and the slot after the table
 * load stay nops. Its only in-image reference is the word 0x801CEBFC at
 * 0x801CFB38, the fifteenth entry of the run at 0x801CFAFC, which this
 * dispatcher's own base reaches at byte-2 index 10, so the overlay loader that
 * steps that run is what calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTablefb10Plchar, D_801CFB10);
