#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the words from 0x800C1F9C through 0x800C208F are
 * the payload's reviewed `textbin` row D_800C1F9C, holding in-image code
 * pointers for this payload's functions and the shared main-executable
 * handlers at 0x801E..... The run is only referenced here. This dispatcher
 * indexes it from the word 0x800C2054, so its base entry is 0x800C1C9C and the
 * twelve reachable entries are 0x800C1C9C, 0x801E52E8, 0x801E35D8, 0x801E3638,
 * 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64,
 * 0x801E52E8 and 0x801E5288. The next word, 0x800C2084, begins the sibling
 * table of the dispatcher at 0x800C1D00.
 */
extern void (*D_800C2054[])(void); /* @source 0x800C2054 @kind data */

/* @source 0x800C1C58
 * @behavior Overlay state dispatch: loads the scratchpad work pointer cell at
 * 0x1F800044, takes that work record's dispatch byte at offset 0x01 as the
 * index and calls the handler pointer at that index of the local handler table
 * based at 0x800C2054. The selected handler's $v0 is left untouched by the
 * epilogue, so this shim's result is whatever that handler returns. The
 * original does not bound the index before the scaled table load, and the slots
 * after the index byte and after that load stay nops. The pointer-cell load
 * precedes frame creation; the 0x18-byte frame only keeps $ra at 0x10 across
 * the indirect call. The payload contains no word or jal site naming
 * 0x800C1C58, so its caller is outside this overlay. Takes no arguments and
 * forwards none. Same shape as the already-exact sibling
 * dispatchStateByte1Table2d24BossBoss01816_800C1B84 of boss018/16, which reads
 * the same dispatch byte through its own table.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table2054BossBoss00216_800C1C58, D_800C2054);
