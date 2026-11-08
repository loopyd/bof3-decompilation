#include "bof3/bof3.h"

/*
 * Code pointer run at 0x800C27B4, the nine-entry work-state table this
 * dispatcher indexes: entry 0 = 0x800C1ED8, entry 1 = 0x800C1F38,
 * entry 2 = 0x800C1F88, entry 3 = 0x800C20CC, entry 4 = 0x800C2310,
 * entry 5 = 0x800C244C, entry 6 = 0x800C24DC, entry 7 = 0x800C25D4,
 * entry 8 = 0x800C26A4. It is an interior word of the 0x800C2744 textbin row,
 * which the reviewed Splat layout places whole, so only the indexed word is
 * named here.
 */
extern void (*D_800C27B4[])(void); /* @source 0x800C27B4 @kind data */

/* Work-record pointer cell in the scratchpad. The original re-materialises
 * this cell for the dispatch index and again after the indirect call, so it is
 * read through the named target symbol instead of the constant-address
 * accessor. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller; this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x800C1E70
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x800C27B4 (0x800C1ED8,
 * 0x800C1F38, 0x800C1F88, 0x800C20CC, 0x800C2310, 0x800C244C, 0x800C24DC,
 * 0x800C25D4, 0x800C26A4) with no arguments. After that call returns it
 * re-reads the pointer cell and, when the work record's unsigned byte at offset
 * 0 is non-zero, runs the shared main-executable service func_8014D290. The
 * original does not bound the index before the scaled table load, and the slots
 * after the index byte, after the table load and in the branch delay slot stay
 * nops. It writes no other state; the 0x18-byte frame only keeps $ra at 0x10
 * across the calls, and this shim returns whatever the guard leaves in $v0.
 * This function's own address is entry 0 of the byte-1 handler run based at
 * 0x800C27AC that dispatchStateByte1Table27acBossBoss03316_800C1E2C indexes.
 * Same shape as dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkByte0Set
 * BmagicMagic00800_801F0294, which additionally copies its table into its own
 * frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkByte0SetBossBoss03316_800C1E70(
    void) {
  D_800C27B4[D_1F800044[2]]();
  if (D_1F800044[0] != 0u) {
    func_8014D290();
  }
}
