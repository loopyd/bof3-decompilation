#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D0988: thirty code pointers, entry 0 =
 * 0x801CE448 through entry 29 = 0x801CFED8, holding this payload's functions plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 20. The target's
 * Splat `textbin` row of the same name places the words, so the run is only
 * referenced here. This dispatcher indexes it from 0x801D09DC, so its base is
 * entry 21 and the reachable entries are 0x801CFF1C through 0x801CFED8. */
extern void (*D_801D09DC[])(void); /* @source 0x801D09DC @kind data */

/* @source 0x801CFED8
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 2, scales it by four and calls
 *           the matching entry of this overlay's handler run based at 0x801D09DC
 *           with no arguments, discarding any result. The original does not bound
 *           the index before the scaled table load. It reads no other state and
 *           writes none; the pointer-cell load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slot after the index byte and the slot after the table load stay
 *           nops. Its only in-image reference is the word 0x801CFED8 at
 *           0x801D09FC, entry 29 of the thirty-entry code-pointer run at
 *           0x801D0988, which this dispatcher's own run reaches at byte-2 index
 *           8, so the overlay loader that steps that run is what calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Table09dcPlcharPlp26700_801CFED8, D_801D09DC);
