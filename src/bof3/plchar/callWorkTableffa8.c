#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFFA8: sixteen code pointers, the words
 * 0x801CFFA8 through 0x801CFFE4, holding this payload's step handlers plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 8 and 13. The reviewed
 * Splat layout places all sixteen words in the one `textbin` row at 0x801CFFA8,
 * so the run is only referenced here. This dispatcher indexes the run from its
 * first word, so its base is entry 0 and the entries it can reach are entry 0 =
 * 0x801CE448 through entry 15 = 0x801CFD40. */
extern void (*codePointerTableffa8[])(void); /* @source 0x801CFFA8 @kind data */

/* Seeded from the lift of the same body at plp349/00 0x801CE404
 * (dispatchWorkByte2Tablef5ecPlcharPlp34900_801CE404), whose metadata recorded
 * it exact. That payload's 68 bytes at offset 4 agree with this payload's 68
 * bytes at offset 4 in sixteen of seventeen words: the only difference is the
 * low half of the scaled table load at 0x801CE428, 0xFFA8 here against 0xF5EC
 * there, because each body names the word of its own run and both runs sit in
 * the one 0x801D page. Statement order, the macro spelling and the declaration
 * style were carried over with this function's own table symbol and name.
 */
/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CFFA8 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load, and that table
 *           load reads the word at the run's base, so entry 0 is 0x801CE448. It
 *           reads no other state and writes none; the pointer-cell load precedes
 *           frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CE404 at 0x801CFFEC, entry 1 of the seven-word pointer run at
 *           0x801CFFE8 that this payload's two work-halfword 0x2C dispatchers
 *           index from the interior bases 0x801CFFEC and 0x801CFFF8; the base
 *           0x801CFFEC is the one that covers it, at work halfword 0x2C = 1. It
 *           has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTableffa8, codePointerTableffa8);
