#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D078C: twenty-four code pointers, the words
 * 0x801D078C through 0x801D07E8, holding this payload's own step functions plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 21. The reviewed
 * Splat layout places all twenty-four words in the one `textbin` row at
 * 0x801D078C, so the run is only referenced here. This dispatcher indexes the run
 * from its first word, so its base is entry 0 and the entries it can reach are
 * entry 0 = 0x801CE448 through entry 23 = 0x801D006C. */
extern void (*func_801D078C[])(void); /* @source 0x801D078C @kind data */

/* Seeded from the lift of the same body at plp349/00 0x801CE404, which measured
 * exact before this seed was written. That payload's 68 bytes at offset 4 agree
 * with this one word for word except the table-base immediate pair in the
 * address materialisation and the scaled table load, where each body names its
 * own run, so the statement order, the macro spelling and the declaration style
 * were carried over with this function's own table symbol and name.
 */
/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801D078C with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. No jal in this payload and no word of this
 *           payload or of the shared executable equals 0x801CE404, so no caller
 *           is named here and the body takes no parameters.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTable078, func_801D078C);
