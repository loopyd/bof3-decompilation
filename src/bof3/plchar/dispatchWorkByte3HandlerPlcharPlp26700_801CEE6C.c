#include "bof3/emi/dispatchers.h"

/* This overlay's second handler table: the entry run starting at 0x801D09B0 of
 * the thirty-word code pointer table 0x801D0988 through 0x801D09FC, which the
 * target's `textbin` row of the same base places. This dispatcher indexes it as
 * entry 10 of that table, so only the words from 0x801D09B0 on are reachable
 * here; the first of them is the step handler 0x801CEEB0. */
extern void (*D_801D09B0[])(void); /* @source 0x801D09B0 @kind data */

/* @source 0x801CEE6C
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 3, scales it by four and calls
 *           the matching entry of this overlay's second handler table based at
 *           0x801D09B0 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled load. It reads no other
 *           state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3HandlerPlcharPlp26700_801CEE6C, D_801D09B0);
