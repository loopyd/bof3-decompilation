#include "bof3/emi/dispatchers.h"

/* This overlay's handler run inside the reviewed `textbin` row stateStepDataTable, which
 * spans 0x800C26B8 through 0x800C277B. The row's first five words are a small
 * record (0x00000101, 0x02020101, 0x03030000, 0x00060006, 0xFFFFFFFF); this
 * dispatcher scales into the run from its word 5, 0x800C26CC, so its entry 0 is
 * this payload's next body 0x800C1860, entries 1 through 5 are the shared
 * main-executable handlers 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980 and
 * 0x801E3A00, and entry 6 is this payload's own byte-2 dispatcher 0x800C18B8. The
 * words from there to 0x800C2774 are the runs this payload's other dispatchers
 * index from their own interior words, and the last word of the row, 0x800C2778,
 * is zero. The row stays one map entry, so this declaration binds only the word
 * the scaled index is added to.
 */
extern void (*D_800C26CC[])(void); /* @source 0x800C26CC @kind data */

/* @source 0x800C181C
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044, takes
 * that record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at the interior word 0x800C26CC with no
 * arguments. The index is bounded only by the zero-extended byte, never compared
 * against the run length. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across
 * the indirect call, and the slots after the index byte load and after the table
 * load stay nops. The epilogue leaves the selected handler's $v0 untouched, so this
 * shim returns whatever that handler returns. No word or jal in this payload names
 * 0x800C181C and it is no entry of the handler row either, so the overlay loader is
 * what calls it. The 68-byte body is byte-identical to the exact lift at 0x800C1CD8
 * of emi/boss/boss008/16, which indexes the same 0x800C26CC run with the same
 * record byte; that span equality was proven by locating all 68 bytes in that
 * payload at offset 0x4D8, and the lift-clone tool declined that representative
 * because its file holds a standalone template rather than an ordinary function, so
 * the statement order here is transcribed from it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      dispatchByte1Table26cc,
                      D_800C26CC);
