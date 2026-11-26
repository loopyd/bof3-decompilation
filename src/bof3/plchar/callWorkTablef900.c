#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF900: seventeen code pointers, the words
 * 0x801CF900 through 0x801CF940, holding this payload's own step handlers plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 7. The reviewed
 * Splat layout places all seventeen words in the one `textbin` row at 0x801CF900,
 * so the run is only referenced here. This dispatcher indexes the run from its
 * first word, so its base is entry 0 and the entries it can reach are entry 0 =
 * 0x801CE448 through entry 16 = 0x801CF868. */
extern void (*codePointerTable[])(void); /* @source 0x801CF900 @kind data */

/* Seeded from the exact lift of the same body at plp457/00 0x801CE404
 * (dispatchWorkByte2Tablef900PlcharPlp45700_801CE404): the two payloads share the
 * load address 0x801CE400 and this payload's 68 body bytes at offset 4 compare
 * equal to that payload's, so both bodies name the one table address 0x801CF900
 * in their own overlays. Statement order, the macro spelling and the declaration
 * style carry over unchanged; this file writes its own emitted name. */
/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CF900 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load, and that table
 *           load reads the word at the run's base, so entry 0 is 0x801CE448. It
 *           reads no other state and writes none; the pointer-cell load precedes
 *           frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CE404 at 0x801CF92C, entry 11 of the seventeen-word code-pointer
 *           run at 0x801CF900, which this payload's work-halfword 0x2C dispatcher
 *           at 0x801CF878 indexes from that interior base, so it is reached at
 *           work halfword 0x2C = 0 and has no jal caller. It takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTablef900, callWorkTablef900, codePointerTable);
