#include "bof3/emi/dispatchers.h"

/* Overlay step dispatcher at 0x800C1B40.
 *
 * The dispatcher selects its handler from the code-pointer run held by this
 * overlay's reviewed `textbin` row D_800C217C, which spans 0x800C217C through
 * the end of the payload at 0x800C227C. The word the scaled index is added to
 * is the table base 0x800C221C, the row's word 40, so the run stays one row and
 * this declaration binds only that word. Its six words run from 0x800C221C up
 * to 0x800C2230, the last word before the next run base 0x800C2234, and hold
 * this payload's own function 0x800C1B84 followed by the main-executable
 * handlers 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980 and 0x801E3A00:
 *
 *   0x800C1B84 0x801E52E8 0x801E35D8 0x801E3638 0x801E3980 0x801E3A00
 *
 * Unlike the byte-2 dispatchers of this overlay, no word of the payload holds
 * 0x800C1B40, so its caller is outside this payload. Takes no arguments.
 */
extern void (*D_800C221C[])(void); /* @source 0x800C221C @kind data */

/* @source 0x800C1B40
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C221C with no arguments.
 * The chosen entry is read from the table itself, not from a frame copy. The
 * original does not bound the index before the scaled table load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, the slots
 * after the index byte and after the table load stay nops, and the epilogue
 * leaves the handler's $v0 untouched, so this shim returns whatever that handler
 * returns. Seeded from the sibling dispatcher
 * dispatchStateByte2Table2204BossBoss00113_800C1980, which measured exact when
 * this body was written: a payload comparison of the two 0x44-byte spans
 * differs in two bytes, the work-record byte offset and the low half of the
 * table load, so the spans share one body and differ only in the record byte
 * this function indexes and the table address their entries point at.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table221cBossBoss00113_800C1B40, D_800C221C);
