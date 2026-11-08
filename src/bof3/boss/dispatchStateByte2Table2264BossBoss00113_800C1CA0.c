#include "bof3/emi/dispatchers.h"

/* Overlay step dispatcher at 0x800C1CA0.
 *
 * The dispatcher selects its handler from the code-pointer run held by this
 * overlay's reviewed `textbin` row D_800C217C, which spans 0x800C217C through
 * the end of the payload at 0x800C227C. The word the scaled index is added to
 * is the table base 0x800C2264, the row's word 58, so the run stays one row and
 * this declaration binds only that word. Its six words hold this payload's own
 * functions 0x800C1CE4, 0x800C1D24, 0x800C1DE0, 0x800C1E38, 0x800C1E40 and
 * 0x800C1E48, all handlers of this overlay. The word this run points at
 * 0x800C1CA0, the dispatcher itself, is a table entry of an enclosing step
 * list rather than a call site.
 */
extern void (*D_800C2264[])(void); /* @source 0x800C2264 @kind data */

/* @source 0x800C1CA0
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C2264 with no arguments.
 * The chosen entry is read from the table itself, not from a frame copy. The
 * original does not bound the index before the scaled table load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, the slots
 * after the index byte and after the table load stay nops, and the epilogue
 * leaves the handler's $v0 untouched, so this shim returns whatever that handler
 * returns. No word and no call in this payload names 0x800C1CA0, so its caller
 * stays outside this overlay. Same shape as the exact sibling
 * dispatchStateByte2Table224cBossBoss00113_800C1BDC; only the table address in
 * the load word and the payload-local entries differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table2264BossBoss00113_800C1CA0, D_800C2264);
