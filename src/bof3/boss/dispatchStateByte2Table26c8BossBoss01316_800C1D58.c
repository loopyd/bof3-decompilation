#include "bof3/emi/dispatchers.h"

/* Overlay step dispatcher at 0x800C1D58.
 *
 * The dispatcher selects its handler from the code-pointer run held by this
 * overlay's reviewed `textbin` row D_800C25D8 (0x800C25D8 through the zero word
 * 0x800C26E0). The word the scaled index is added to is the table base
 * 0x800C26C8, the row's word 60, so the run stays one row and this declaration
 * binds only that word. The entries reachable from it are this payload's own
 * functions 0x800C1D9C, 0x800C1DF4, 0x800C1E90, 0x800C1F24 and 0x800C2004
 * interleaved with the main-executable handler 0x801E51DC, with the zero word
 * last. This function's own address is the row's word 57 at 0x800C26BC and no
 * jal or other word in this payload names it, so the overlay loader that steps
 * that row is its caller.
 */
extern void (*D_800C26C8[])(void); /* @source 0x800C26C8 @kind data */

/* @source 0x800C1D58
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C26C8 with no arguments.
 * The original does not bound the index before the scaled table load. It reads
 * no other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, the slots
 * after the index byte and after the table load stay nops, and the epilogue
 * leaves the handler's $v0 untouched, so this shim returns whatever that handler
 * returns. The 68-byte body is instruction-for-instruction identical to the
 * already-exact sibling dispatchStateByte1Table2698BossBoss01316_800C1CC4 and to
 * dispatchStateByte2Table1ea4BossBoss01216_800C18B4, differing only in the
 * work-record byte that indexes the table and the %lo table displacement of the
 * scaled-index load.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table26c8BossBoss01316_800C1D58, D_800C26C8);
