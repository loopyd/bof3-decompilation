#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFD80: sixteen code pointers, the words
 * 0x801CFD80 through 0x801CFDBC, entry 0 = 0x801CE448 through entry 15 =
 * 0x801CF938, holding this payload's own functions plus the shared
 * main-executable handler 0x801B2D54 at entries 2, 8 and 13. The reviewed Splat
 * layout places all sixteen words in the one `textbin` row at 0x801CFD80, so the
 * run is only referenced here. This dispatcher indexes the run from its first
 * word, so its base is entry 0 and the entries it can reach are entry 0 through
 * entry 15 = 0x801CF938. */
extern void (*T_801CFD80[])(void); /* @source 0x801CFD80 @kind data */

/* Seeded from the overlay lift of the same body at 0x801CE404 whose own row is
 * the 0x801CF900 handler run: its 68 bytes measured exact, and the two spans
 * agree byte for byte except the low half of the scaled table-load word at
 * 0x801CE428, where each body names the word of the run it indexes. Statement
 * order, the macro spelling and the declaration style were carried over with
 * this function's own table symbol.
 */
/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CFD80 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load, and that load
 *           reads the word at the run's base, so entry 0 is 0x801CE448. It reads
 *           no other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CE404 at 0x801CFDC0, entry 0 of the six-word pointer run at
 *           0x801CFDC0 that this target's Splat declares as a `textbin` row, so
 *           the overlay loader that steps that run is what enters it: it has no
 *           jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTablefd80Plchar, T_801CFD80);
