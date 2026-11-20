#include "bof3/emi/dispatchers.h"

/* Overlay step dispatcher at 0x800C1980.
 *
 * The dispatcher selects its handler from the code-pointer run held by this
 * overlay's reviewed `textbin` row D_800C217C, which spans 0x800C217C through
 * the end of the payload at 0x800C227C. The word the scaled index is added to
 * is the table base 0x800C2204, the row's word 34, so the run stays one row and
 * this declaration binds only that word. Its three words run from 0x800C2204 up
 * to 0x800C220C, the last word before the next run base 0x800C2210, and hold
 * this payload's own functions 0x800C19C4, 0x800C1A08 and 0x800C1AD0.
 *
 * The word at 0x800C21E8 holds 0x800C1980, so this shim is one entry of an
 * enclosing step list in that same run; no jal in this payload calls it.
 */
extern void (*D_800C2204[])(void); /* @source 0x800C2204 @kind data */

/* @source 0x800C1980
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C2204 with no arguments.
 * The chosen entry is read from the table itself, not from a frame copy. The
 * original does not bound the index before the scaled table load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, the slots
 * after the index byte and after the table load stay nops, and the epilogue
 * leaves the handler's $v0 untouched, so this shim returns whatever that handler
 * returns. Seeded from the sibling dispatcher
 * dispatchStateByte2Table21ecBossBoss00113_800C18BC, which measured exact when
 * this body was written: a payload comparison of the two 0x44-byte spans
 * differed in the two bytes of one word, the low half of the table load, so the
 * spans share one body and differ only in the table address and the
 * payload-local entries it points at.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table2204BossBoss00113_800C1980, D_800C2204);
