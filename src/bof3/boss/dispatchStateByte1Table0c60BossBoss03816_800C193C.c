#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x800C1C60: twelve code pointers inside the
 * reviewed `textbin` row D_800C1C10, holding this payload's own func_800C1980
 * followed by the shared main-executable handlers 0x801E52E8, 0x801E35D8,
 * 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C,
 * 0x801E4F64, 0x801E52E8 and 0x801E5288; the row's last pointer run at
 * 0x800C1C90 follows. Splat places that whole row, so it is only referenced
 * here; this dispatcher's base is the run's first word. */
extern void (*D_800C1C60[])(void); /* @source 0x800C1C60 @kind data */

/* @source 0x800C193C
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044, takes
 * that record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x800C1C60 with no arguments. The
 * original does not bound the index before the scaled table load, and the slots
 * after the index byte and after the table load stay nops. It reads no other state
 * and writes none; the pointer-cell load precedes frame creation, the 0x18-byte
 * frame only keeps $ra at 0x10 across the indirect call, and the selected handler's
 * $v0 is left untouched by the epilogue, so this shim returns whatever that handler
 * returns. No word or `jal` in this payload names 0x800C193C, so its caller is
 * outside this overlay. Same shape as the already-exact siblings
 * dispatchStateByte1Table2744BossBoss00816_800C1EB0 and
 * dispatchStateByte2Table1df4BossBoss05116_800C18BC, which read the same work
 * record through their own tables.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table0c60BossBoss03816_800C193C, D_800C1C60);
