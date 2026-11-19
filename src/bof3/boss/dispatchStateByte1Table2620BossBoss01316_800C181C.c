#include "bof3/emi/dispatchers.h"

/* This overlay's handler run inside the reviewed `textbin` row D_800C25D8. The
 * run starts at the row's word 18, 0x800C2620, and the twelve words from there to
 * 0x800C264C are the base this dispatcher scales into: entry 0 is this payload's
 * next body 0x800C1860 and entries 1 through 11 are the shared main-executable
 * handlers 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00,
 * 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288.
 * The next dispatcher of this payload indexes from 0x800C2650, the word after
 * that span. Several dispatchers of this payload index the same row from their own
 * interior word, so Splat places the row once and it is only referenced here. */
extern void (*D_800C2620[])(void); /* @source 0x800C2620 @kind data */

/* @source 0x800C181C
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044, takes
 * that record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x800C2620 with no arguments. The
 * index is bounded only by the zero-extended byte, never compared against the run
 * length, so the scaled load can leave the twelve words of that base. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation, the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the slots
 * after the index byte load and after the table load stay nops. The epilogue
 * leaves the selected handler's $v0 untouched, so this shim returns whatever that
 * handler returns. No word or jal in this payload names 0x800C181C and it is no
 * entry of the handler row either, so the overlay loader is what calls it. The
 * 68-byte body is the same instruction sequence as the sibling at 0x800C1A34 up to
 * the %lo table displacement of the scaled-index load at instruction 9, which
 * names each body's own table.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      dispatchStateByte1Table2620BossBoss01316_800C181C,
                      D_800C2620);
