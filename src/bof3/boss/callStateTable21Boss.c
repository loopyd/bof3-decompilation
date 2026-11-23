#include "bof3/emi/dispatchers.h"

/* Overlay step dispatcher at 0x800C1820.
 *
 * The 0x1C bytes before this function at 0x800C1804 are not code: they are this
 * overlay's seven-entry mode table, which the reviewed layout places as its own
 * `textbin` row. The mode dispatcher func_800C1E88 masks its byte index to 0xFF,
 * rejects indices of seven and above with `sltiu $v0,$a0,0x7`, scales the rest
 * by four and jumps through the word it loads from 0x800C1804, so that run is
 * one table base and this body starts at 0x800C1820.
 *
 * The dispatcher selects its handler from the code-pointer run held by this
 * overlay's reviewed `textbin` row D_800C217C, which spans 0x800C217C through
 * the end of the payload at 0x800C227C. The word the scaled index is added to
 * is the table base 0x800C21BC, the row's word 16, so the run stays one row and
 * this declaration binds only that word. Its six words run from 0x800C21BC up
 * to 0x800C21D0, the last word before the next run base 0x800C21D4, and hold
 * this payload's own function 0x800C1864 followed by the main-executable
 * handlers 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980 and 0x801E3A00:
 *
 *   0x800C1864 0x801E52E8 0x801E35D8 0x801E3638 0x801E3980 0x801E3A00
 *
 * The word at 0x800C21D4 holds 0x800C18BC, this payload's own byte-2
 * dispatcher, so that shim is an entry of an enclosing step list in the same
 * run. No word of the payload holds 0x800C1820, so its caller is outside this
 * payload.
 */
extern void (*D_800C21BC[])(void); /* @source 0x800C21BC @kind data */

/* @source 0x800C1820
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C21BC with no arguments.
 * The chosen entry is read from the table itself, not from a frame copy. The
 * original does not bound the index before the scaled table load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, the slots
 * after the index byte and after the table load stay nops, and the epilogue
 * leaves the handler's $v0 untouched, so this shim returns whatever that handler
 * returns. A payload comparison of this 0x44-byte span against the exact sibling
 * dispatchStateByte1Table221cBossBoss00113_800C1B40 differs in one word, the low
 * half of the scaled table load, so the two spans share one body and differ only
 * in the table address those entries point at.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateTable21Boss, D_800C21BC);
