#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the words from 0x800C275C through 0x800C27D4 are
 * in-image code pointers, this payload's functions interleaved with the shared
 * main-executable handlers at 0x801E.... The run sits inside the 0x800C2744
 * `textbin` row, whose first 24 bytes hold the two byte tables func_800C1848
 * hands to its callees; the reviewed Splat layout places that whole row, so it
 * is only referenced here. This dispatcher indexes the run from its word at
 * 0x800C27AC, so its base entry is 0x800C1E70 and the eleven reachable entries
 * are 0x800C1E70, 0x800C256C, 0x800C1ED8, 0x800C1F38, 0x800C1F88, 0x800C20CC,
 * 0x800C2310, 0x800C244C, 0x800C24DC, 0x800C25D4 and 0x800C26A4. */
extern void (*D_800C27AC[])(void); /* @source 0x800C27AC @kind data */

/* @source 0x800C1E2C
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044,
 * takes that record's unsigned byte at offset 1, scales it by four and calls the
 * matching entry of this overlay's handler run based at 0x800C27AC with no
 * arguments. The original does not bound the index before the scaled table load,
 * and the slots after the index byte and after the table load stay nops. It
 * reads no other state and writes none; the pointer-cell load precedes frame
 * creation, the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 * and the selected handler's $v0 is left untouched by the epilogue, so this shim
 * returns whatever that handler returns. No word or jal in this payload names
 * 0x800C1E2C, so its caller is outside this overlay. Same shape as the
 * already-exact siblings dispatchStateByte1Table2d24BossBoss01816_800C1B84,
 * which also indexes an interior word of its own pointer run, and
 * dispatchStateByte1Table2744BossBoss00816_800C1EB0.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table27acBossBoss03316_800C1E2C, D_800C27AC);
