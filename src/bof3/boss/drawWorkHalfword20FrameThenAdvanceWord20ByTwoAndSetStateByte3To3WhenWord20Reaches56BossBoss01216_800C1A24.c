#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell, bound by its own D_ name and not by
 * SPAD_PTR_SLOT: this body materializes the base itself for each of its three
 * accesses (lui $v0,0x1F80 / lw $v0,68($v0), then again through $v0 and through
 * $a0), while the macro's absolute-constant lvalue makes the compiler hold
 * 0x1F800044 in a callee-saved register and load through offset zero. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* This payload's own render helpers at 0x800C1C08 and 0x800C1CD8, still on
 * their `asm` rows: each takes one unsigned halfword, the texture V coordinate
 * it stamps into the primitive it emits (0x800C1C08 draws through SetSprt,
 * 0x800C1CD8 through SetPolyFT4). No `andi` narrows the argument at either call
 * site, so the parameter is the halfword the caller loads. */
extern void func_800C1C08(u16 arg0);
extern void func_800C1CD8(u16 arg0);

/* @source 0x800C1A24
 * @behavior Overlay frame step, entry 2 of this payload's byte-3 handler run
 *   based at 0x800C1EBC: it loads the work-record pointer published at the
 *   scratchpad pointer cell 0x1F800044 and passes that record's unsigned
 *   halfword at 0x20 to the render helper func_800C1C08, re-reads the pointer
 *   cell and passes the same halfword to func_800C1CD8, then re-reads the cell
 *   once more, adds two to the record's word at 0x20, stores the sum back and,
 *   when that sum equals 0x56, writes 3 into the record's byte at offset 3, the
 *   byte the byte-3 dispatcher dispatchStateByte3Table1ebcBossBoss01216_800C18F8
 *   indexes its own run with; 0x800C1A90, entry 3 of this run, is the body that
 *   is reached next once byte 3 is 3. Both render calls take the counter at 0x20
 *   as it stands before this step, which is why the original reads it three
 *   times, twice as an unsigned halfword and once as the word the increment
 *   works on. The 0x18-byte frame only keeps $ra at 0x10 across the two calls,
 *   each pointer-cell access materializes its own lui/lw pair, and the store to
 *   word 0x20 sits in the bne delay slot, so both paths advance the counter. The
 *   function reads no other state, returns nothing, and installs nothing. No jal
 *   of this payload reaches it: its address is the single word 0x800C1A24 at
 *   0x800C1EC4, entry 2 of the run, so the byte-3 dispatcher enters it from
 *   outside the payload and its argument roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation: the two render arguments are the cell expression
 * `*(u16 *)(D_1F800044 + 0x20)` written at the call site, not a named halfword
 * local. Loading a local first, whether one reused `u16` (21/27), two `u16`
 * locals (21/27) or a `u32` local narrowed at the call (21/27), let the
 * allocator coalesce the pointer temp into the argument register, so the base
 * came out as `lui $a0` / `lw $a0` and the halfword as `lhu $a0,0x20($a0)` where
 * the original has the base in $v0 and the halfword in $a0. The inline cell form
 * keeps the pointer a separate temp and measures 27/27 instructions with no
 * differing byte. The word 0x20 step keeps its own `u32` local: the original
 * compares the incremented value, so a re-read of the cell there costs the
 * reload the original does not have.
 */
void drawWorkHalfword20FrameThenAdvanceWord20ByTwoAndSetStateByte3To3WhenWord20Reaches56BossBoss01216_800C1A24(
    void) {
  u8 *work;
  u32 counter;

  func_800C1C08(*(u16 *)(D_1F800044 + 0x20));
  func_800C1CD8(*(u16 *)(D_1F800044 + 0x20));
  work = D_1F800044;
  counter = *(u32 *)(work + 0x20) + 2;
  *(u32 *)(work + 0x20) = counter;
  if (counter == 0x56) {
    work[3] = 3;
  }
}
