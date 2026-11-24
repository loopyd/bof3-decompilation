#include "bof3/emi/dispatchers.h"

/* Overlay step dispatcher at 0x800C18BC.
 *
 * The dispatcher selects its handler from the code-pointer run held by this
 * overlay's reviewed `textbin` row D_800C217C, which spans 0x800C217C through
 * the end of the payload at 0x800C227C. The word the scaled index is added to
 * is the table base 0x800C21EC, the row's word 28, so the run stays one row and
 * this declaration binds only that word. Its six words run from 0x800C21EC up
 * to 0x800C2200, the last word before the next run base, and hold the
 * main-executable handlers 0x801E3BAC, 0x801E3BD0, 0x801E4490 and 0x801E4928
 * interleaved with this payload's own function 0x800C1900:
 *
 *   0x801E3BAC 0x801E3BD0 0x801E3BAC 0x801E4490 0x800C1900 0x801E4928
 *
 * The word at 0x800C21D4 holds 0x800C18BC, so this shim is one entry of an
 * enclosing step list in the same run; no jal in this payload calls it.
 */
extern void (*D_800C21EC[])(void); /* @source 0x800C21EC @kind data */

/* @source 0x800C18BC
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C21EC with no arguments.
 * The chosen entry is read from the table itself, not from a frame copy. The
 * original does not bound the index before the scaled table load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, the slots
 * after the index byte and after the table load stay nops, and the epilogue
 * leaves the handler's $v0 untouched, so this shim returns whatever that handler
 * returns. Seeded from the sibling dispatcher
 * callStateTable2264Boss, which was exact when this
 * body was written: a payload comparison of the two 0x44-byte spans differs in
 * the two bytes of one word, the low half of the table load, so the spans share
 * one body and differ only in the table address and the payload-local entries
 * it points at.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callStateByte2Table1, D_800C21EC);
