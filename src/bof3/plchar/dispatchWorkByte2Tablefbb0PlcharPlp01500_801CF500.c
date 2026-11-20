#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFB84: sixteen code pointers, entry 0 =
 * 0x801CE448 through entry 15 = 0x801CF91C, holding this payload's step
 * functions plus the shared main-executable handler 0x801B2D54 at entries 2, 8
 * and 13. The Splat `textbin` row of the same name places the words, so the run
 * is only referenced here. This dispatcher indexes the run from its word 11, so
 * its base sits at 0x801CFBB0 and the entries it can reach are 0x801CF544
 * through 0x801CF91C. */
extern void (*D_801CFBB0[])(void); /* @source 0x801CFBB0 @kind data */

/* @source 0x801CF500
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CFBB0 with no arguments, discarding any result. That base is
 *           entry 11 of the sixteen-entry run at 0x801CFB84, so the walk can
 *           reach entries 11 through 15, 0x801CF544 through 0x801CF91C. The
 *           original does not bound the index before the scaled table load. It
 *           reads no other state and writes none; the pointer-cell load precedes
 *           frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CF500 at 0x801CFBD0, entry 3 of the seven-entry code-pointer run
 *           at 0x801CFBC4, which the sibling dispatcher at 0x801CFAFC indexes
 *           from 0x801CFBC8, so that dispatcher and the overlay loader that steps
 *           the run are what enter it: no jal target, no other literal word
 *           naming it in this payload, and no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tablefbb0PlcharPlp01500_801CF500, D_801CFBB0);
