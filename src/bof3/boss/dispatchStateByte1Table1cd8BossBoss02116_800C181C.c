#include "bof3/emi/dispatchers.h"

/* This overlay's handler run starts at 0x800C1CD8, the first code-pointer word of
 * the 184-byte `textbin` row D_800C1CB0 that Splat places after this payload's
 * code; the ten words before it are byte-sized game data, not pointers. From
 * 0x800C1CD8 the run starts with this payload's step publisher 0x800C1860, then
 * the shared main-program handlers 0x801E52E8, 0x801E35D8, 0x801E3638,
 * 0x801E3980, 0x801E3A00, this payload's 0x800C18B8, the main-program handlers
 * 0x801E4AE8, 0x801E4D8C and 0x801E4F64, and this payload's 0x800C18FC and
 * 0x800C19C4 further down the row. Splat places the whole row, so this
 * declaration names only the interior word the scaled index is added to.
 */
extern void (*D_800C1CD8[])(void); /* @source 0x800C1CD8 @kind data */

/* @source 0x800C181C
 * @behavior State dispatch on the work record byte at offset 1: loads the
 * work-record pointer published at the scratchpad pointer cell 0x1F800044 before
 * creating its 0x18-byte frame, takes that record's unsigned byte at offset 1,
 * scales it by four and calls the matching entry of this overlay's handler run
 * based at 0x800C1CD8 with no arguments. The original bounds the index nowhere
 * before the scaled table load, and the slots after the index byte and after the
 * table load stay nops. It reads no other state and writes none: the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across
 * the indirect call, and the epilogue leaves the selected handler's $v0
 * untouched, so this shim returns whatever that handler returns. The byte it
 * indexes with is the one the step publisher at 0x800C1860 stores 2 into, and
 * word 0 of the run is that same publisher, so writing 2 selects the main-program
 * handler 0x801E35D8 with the table this body reads. No word in this payload
 * equals 0x800C181C, no jal targets it and no lui/addiu pair forms it, so neither
 * the handler run nor a call names it and the main program's overlay handler
 * table is what enters it. The 68-byte body is byte-identical to the sibling
 * dispatchStateByte1Table1d2cBossBoss02116_800C19E4 of this same payload, which
 * measured exact, apart from the one word carrying this body's table base: the
 * scaled-load displacement 0x1CD8 here against 0x1D2C there, both under the same
 * 0x800C page. That sibling's source supplied the statement order and the operand
 * forms transcribed here.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      dispatchStateByte1Table1cd8BossBoss02116_800C181C,
                      D_800C1CD8);
