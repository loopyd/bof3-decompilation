#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the words from 0x800C319C through 0x800C32A4 are
 * in-image code pointers, this payload's functions plus shared main-executable
 * handlers, and the word at 0x800C32A8 is not a pointer, so the run ends there.
 * The reviewed Splat layout places those words in the trailing `textbin` row
 * D_800C3174, so the run is only referenced here. This dispatcher indexes it
 * from 0x800C327C, an interior word of the run, and the reachable entries are
 * 0x801E3BAC, 0x801E3BD0, 0x801E3BAC, 0x801E4490, 0x800C2074, 0x801E4928 and
 * then the five-entry 0x800C3294 handler table of the sibling dispatcher at
 * 0x800C20F0: 0x800C2128, 0x800C2160, 0x800C217C, 0x800C3038 and 0x800C3088. */
extern void (*D_800C327C[])(void); /* @source 0x800C327C @kind data */

/* @source 0x800C2030
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 2, scales it by four and calls
 *           the matching entry of this overlay's handler run based at 0x800C327C
 *           with no arguments. The original does not bound the index before the
 *           scaled load. It reads no other state and writes none; the pointer-cell
 *           load precedes frame creation, the 0x18-byte frame only keeps $ra at
 *           0x10 across the indirect call, and the slot after the index byte and
 *           the slot after the table load stay nops. Its only in-image reference is
 *           the word 0x800C2030 at 0x800C325C, entry 6 of the run based at
 *           0x800C3244, so no `jal` reaches it and it can only be entered from
 *           outside the overlay. Same body as the sibling dispatchers of this
 *           overlay, which read work-record byte 1 or index another word of the
 *           same run; only the record byte and the table address in the load word
 *           differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table327cBossBoss02513_800C2030, D_800C327C);
