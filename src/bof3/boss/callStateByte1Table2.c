#include "bof3/emi/dispatchers.h"

/*
 * Handler run the byte-1 dispatcher at 0x800C1804 indexes.
 *
 * The twelve words from 0x800C1E74 through 0x800C1EA0 sit inside this overlay's
 * reviewed textbin pointer row D_800C1E14 (0x800C1E14 through the payload end
 * 0x800C1ED8) and hold 0x800C1848, 0x801E52E8, 0x801E35D8, 0x801E3638,
 * 0x801E3980, 0x801E3A00, 0x800C18B4, 0x801E4AE8, 0x801E4D8C, 0x801E4F64,
 * 0x801E52E8 and 0x801E5288. The byte-2 table base 0x800C1EA4 of
 * callByte2Table1ea4 starts at the next word. The
 * row stays one map entry, so this declaration binds only the word the scaled
 * index is added to.
 */
extern void (*D_800C1E74[])(void); /* @source 0x800C1E74 @kind data */

/* @source 0x800C1804
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of the handler run based at 0x800C1E74 with no arguments. The original
 * does not bound the index before the scaled table load, and the slots after the
 * index byte, after the table load and in the branch delay slot stay nops. It
 * writes no state and leaves the handler's $v0 untouched, so this shim returns
 * whatever that handler returns; the pointer-cell load precedes frame creation
 * and the 0x18-byte frame only keeps $ra at 0x10 across the indirect call. No
 * word of this payload holds 0x800C1804, so the caller that reaches this shim
 * sits outside the payload. Seeded from the sibling
 * dispatchStateByte1Table2154BossBoss02816_800C1804 of boss028/16, which was
 * recorded exact at 100.00 when it was lifted: the two 0x44-byte spans compare
 * equal word for word except the scaled table load's low immediate, 0x800C1E74
 * here against 0x800C2154 there, and both addresses sit in the 0x800C page, so
 * even that word's %hi half is shared and only this target's own table binding
 * is written from these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      callStateByte1Table2,
                      D_800C1E74);
