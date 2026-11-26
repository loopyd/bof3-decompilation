#include "bof3/emi/dispatchers.h"

/* Overlay entry dispatcher at 0x800C1814.
 *
 * The dispatcher selects its handler from the pointer run this overlay binds as
 * the reviewed `textbin` row stateDispatchTable (0x800C1DB0 through the end of the
 * payload at 0x800C1E08). The word the scaled index is added to is the table
 * base 0x800C1DC4, the run's word 5, so the run stays one row and this
 * declaration binds only that word. Its entry 0 is this payload's step publisher
 * at 0x800C1858 and entry 2 is the shared handler 0x801E35D8, both reached by
 * the state byte this table is indexed with; entry 1 is 0x801E52E8 and entry 3
 * 0x801E3638, outside this payload.
 */
extern void (*D_800C1DC4[])(void); /* @source 0x800C1DC4 @kind data */

/* @source 0x800C1814
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C1DC4 with no arguments.
 * The original does not bound the index before the scaled load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the
 * epilogue restores $ra and $sp without touching $v0, so this shim returns
 * whatever the selected handler returns. No word of the payload holds
 * 0x800C1814 and no `jal` names it, so the overlay loader enters it after the
 * sixteen-byte handler table at 0x800C1804. The 68-byte body is the same body
 * as this overlay's sibling callStateByte2Table8,
 * which measured 100.00 before this lift and yields this shape: the two bodies
 * differ in exactly two words, the record byte the load reads (offset 1 here,
 * offset 2 there) and the `%lo` immediate of the table load (0x1DC4 here,
 * 0x1DF4 there).
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateByte1Table11, D_800C1DC4);
