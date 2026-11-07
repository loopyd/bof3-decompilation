#include "bof3/emi/dispatchers.h"

/* Overlay step dispatcher at 0x800C1EE4.
 *
 * The reviewed Splat layout places this overlay's in-image pointer run inside
 * the 0x800C2C38 `textbin` row, which runs from 0x800C2C38 to the end of the
 * payload at 0x800C2DD4, so the run is only referenced here. The word the
 * scaled index is added to is the table base 0x800C2D94, an interior word of
 * that run, so the run stays one reviewed row and this declaration binds only
 * that word. The twelve words from 0x800C2D94 are in-image code pointers:
 * 0x801E3BAC, 0x801E3BD0, 0x801E3BAC and 0x801E4490, then this overlay's
 * 0x800C1F28, 0x801E4928, 0x800C203C, 0x800C2064, 0x800C206C, 0x800C20AC,
 * 0x800C20BC and 0x800C210C. The word at 0x800C2DC4 is not an in-image
 * pointer, so the pointer run ends after those twelve entries.
 */
extern void (*D_800C2D94[])(void); /* @source 0x800C2D94 @kind data */

/* @source 0x800C1EE4
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C2D94 with no arguments.
 * The original does not bound the index before the scaled load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the
 * selected handler's $v0 is left untouched by the epilogue, so this shim
 * returns whatever that handler returns. This function's only in-image
 * reference is the run word at 0x800C2D78, so no `jal` reaches it and its
 * caller is outside this overlay. Same shape as the exact siblings
 * dispatchStateByte1Table2d24BossBoss01816_800C1B84 of this overlay and
 * dispatchStateByte2Table1df4BossBoss05116_800C18BC of boss051/16, which read
 * their own work byte through their own tables; only the record byte and the
 * table address in the load word differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table2d94BossBoss01816_800C1EE4, D_800C2D94);
