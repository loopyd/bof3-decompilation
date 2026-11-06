#include "bof3/emi/dispatchers.h"

/* The byte-3 handler run at 0x801D08B0: two code pointers, entry 0 = 0x801D0288
 * and entry 1 = 0x801D02F4, the two step handlers this dispatcher reaches. The
 * Splat `textbin` row at 0x801D0858 places the words of the twenty-four-entry
 * run these two end, so the run is only referenced here; this base is entry 22
 * of it. */
extern void (*D_801D08B0[])(void); /* @source 0x801D08B0 @kind data */

/* @source 0x801D0244
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 3, scales it by four and calls the matching
 * entry of the two-entry handler run based at 0x801D08B0 with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call, and the slot after the index byte and the slot
 * after the table load stay nops. Its only in-image reference is the word
 * 0x801D0244 at 0x801D0898, entry 16 of the twenty-four-entry run at
 * 0x801D0858, so it has no direct caller and the dispatchers over that run are
 * what reach it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3Table08b0PlcharPlp05600_801D0244, D_801D08B0);
