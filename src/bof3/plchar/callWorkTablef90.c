#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF900: seventeen code pointers, the words
 * 0x801CF900 through 0x801CF940, holding this payload's own step handlers plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 7. The reviewed
 * Splat layout places all seventeen words in the one `textbin` row at 0x801CF900,
 * so the run is only referenced here. This dispatcher indexes it from 0x801CF90C,
 * so its base is entry 3 of that run and entry 0 is 0x801CE654. */
extern void (*D_801CF90C[])(void); /* @source 0x801CF90C @kind data */

/* Transcribed from the byte-identical lift at plp457/00 0x801CE620
 * (dispatchWorkByte2Tablef90cPlcharPlp45700_801CE620), whose own gate reported
 * exact. The two payloads share the load address 0x801CE400 and the 68 body bytes
 * at offset 0x210 of this payload compared equal to that payload's span at offset
 * 0x220, so both bodies name the one table base 0x801CF90C. Statement order, the
 * macro spelling and the declaration style carry over unchanged; this file writes
 * its own emitted name. */
/* @source 0x801CE610
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CF90C with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CE610 at 0x801CF938, entry 14 of the seventeen-word code-pointer
 *           run at 0x801CF900, which this payload's work-halfword 0x2C dispatcher
 *           at 0x801CF8BC indexes from that interior base, so it is reached at
 *           work halfword 0x2C = 0 and has no jal caller. It takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTablef90, callWorkTablef90, D_801CF90C);
