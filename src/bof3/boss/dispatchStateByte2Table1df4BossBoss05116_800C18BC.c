#include "bof3/emi/dispatchers.h"

/* Overlay step dispatcher at 0x800C18BC.
 *
 * The dispatcher selects its handler from the pointer run this overlay binds as
 * the reviewed `textbin` row D_800C1DB0 (0x800C1DB0 through the end of the
 * payload at 0x800C1E08). The word the scaled index is added to is the table
 * base 0x800C1DF4, the run's word 17, so the run stays one row and this
 * declaration binds only that word.
 */
extern void (*D_800C1DF4[])(void); /* @source 0x800C1DF4 @kind data */

/* @source 0x800C18BC
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C1DF4 with no arguments.
 * The original does not bound the index before the scaled load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the
 * selected handler's $v0 is left untouched by the epilogue, so this shim
 * returns whatever that handler returns. This function's only in-image
 * reference is the run word at 0x800C1DD4, so no `jal` reaches it. Same shape as
 * the exact sibling dispatchStateByte2Table2690BossBoss00816_800C18A0, which
 * reads the same work byte through its own overlay's table; only the table
 * address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table1df4BossBoss05116_800C18BC, D_800C1DF4);
