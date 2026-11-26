#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFCB4: sixteen code pointers, 0x801CFCB4
 * through 0x801CFCF0, holding this payload's own functions plus the shared
 * main-executable handler 0x801B2D54 at entries 2, 7 and 13. The target's
 * `textbin` row of the same base places the words, so the run is only referenced
 * here. This dispatcher indexes it from 0x801CFCC8, which is entry 5 of that run,
 * so the entries a byte-2 index reaches are the run's entry 5 onwards — the
 * functions at 0x801CEC40, 0x801CED34, 0x801B2D54, 0x801CEE74 and their
 * successors — then the payload's six trailing words at 0x801CFCF4, and past the
 * payload end at 0x801CFD0C. */
extern void (*D_801CFCC8[])(void); /* @source 0x801CFCC8 @kind data */

/* @source 0x801CEBFC
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CFCC8 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CEBFC at 0x801CFCF8, entry 1 of the payload's last six words, so
 *           the overlay loader that steps that run is what enters it: it has no
 *           jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTablefcc8, D_801CFCC8);
