#include "bof3/emi/dispatchers.h"

/* This overlay's handler run inside the reviewed `textbin` row D_800C25D8: this
 * dispatcher indexes from 0x800C265C, the row's word 33 of the 67 words the row
 * holds, so entry 0 is this payload's next body 0x800C1A78. Entries 1 through 11
 * are shared main-executable handlers, entries 12 through 15 are this payload's
 * steps 0x800C1B4C, 0x800C1C4C, 0x800C1CBC and 0x800C1D08, entry 24 and entries 27
 * through 32 are the later steps 0x800C1D58, 0x800C1D9C, 0x800C1DF4, 0x800C1E90,
 * 0x800C1F24 and 0x800C2004 beside more shared handlers, and the zero word
 * 0x800C26E0 at entry 33 ends it. Several dispatchers of this payload index that
 * same row from their own interior word, so Splat places the row once and it is
 * only referenced here. */
extern void (*D_800C265C[])(void); /* @source 0x800C265C @kind data */

/* @source 0x800C1A34
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044, takes
 * that record's unsigned byte at offset 1, scales it by four and calls the
 * matching entry of this overlay's handler run based at 0x800C265C with no
 * arguments. The original does not bound the index before the scaled table load,
 * so the zero word that ends the run stays reachable, and the slots after the
 * index byte and after the table load stay nops. It reads no other state and
 * writes none; the pointer-cell load precedes frame creation, the 0x18-byte frame
 * only keeps $ra at 0x10 across the indirect call, and the selected handler's $v0
 * is left untouched by the epilogue, so this shim returns whatever that handler
 * returns. No word or jal in this payload names 0x800C1A34, and it is no entry of
 * the handler run either, so the overlay loader is what calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      callStateTable265Boss,
                      D_800C265C);
