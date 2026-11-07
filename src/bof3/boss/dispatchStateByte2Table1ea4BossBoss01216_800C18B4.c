#include "bof3/emi/dispatchers.h"

/* Overlay step dispatcher at 0x800C18B4.
 *
 * The dispatcher selects its handler from the code-pointer run held by this
 * overlay's reviewed `textbin` row D_800C1E14 (0x800C1E14 through the end of the
 * payload at 0x800C1ED8). The word the scaled index is added to is the table base
 * 0x800C1EA4, the row's word 36, so the run stays one row and this declaration
 * binds only that word. The reachable entries are the main-executable handlers
 * 0x801E3BAC, 0x801E3BD0, 0x801E4490 and 0x801E4928 interleaved with this
 * payload's own functions 0x800C18F8, 0x800C193C, 0x800C1978, 0x800C1A24,
 * 0x800C1A90, 0x800C1B50, 0x800C1B58 and 0x800C1B60, thirteen words in all.
 */
extern void (*D_800C1EA4[])(void); /* @source 0x800C1EA4 @kind data */

/* @source 0x800C18B4
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C1EA4 with no arguments.
 * The chosen entry is read from the table itself, not from a frame copy. The
 * original does not bound the index before the scaled table load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, the slots
 * after the index byte and after the table load stay nops, and the epilogue
 * leaves the handler's $v0 untouched, so this shim returns whatever that handler
 * returns. This function's only in-image reference is the word 0x800C18B4 at
 * 0x800C1E8C, word 30 of the same code-pointer row, so the overlay loader that
 * steps that row is its caller. Same shape as the exact sibling
 * dispatchStateByte2Table1df4BossBoss05116_800C18BC; only the table address in
 * the load word and the payload-local entries differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table1ea4BossBoss01216_800C18B4, D_800C1EA4);
