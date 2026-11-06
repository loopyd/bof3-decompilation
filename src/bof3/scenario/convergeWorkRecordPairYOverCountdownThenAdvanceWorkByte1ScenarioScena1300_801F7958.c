#include "bof3/bof3.h"

/* @source 0x801468C0 @kind unknown */
extern s32 D_801468C0[];

/* @source 0x801F7958
 * @behavior Entry 9 of this overlay's twelve-word work-object handler table
 * D_801FBEF0 (its word at 0x801FBF14), the table the frame dispatcher
 * func_801F6EA4 selects with byte 0x01 of the scratchpad work object published
 * at pointer slot 0x1F800044. It takes no arguments and returns nothing, and it
 * branches on that object's byte at 0x09: while the byte is non-zero it moves
 * the y word at 0x38 of the two 0x98-byte work records 0x80146888[byte 0x03]
 * and 0x80146888[byte 0x04] by +0x800 and -0x800 and then stores that byte
 * minus one through the same loaded pointer. When byte 0x09 reads zero it
 * stores 0xF into byte 0x09 through that pointer, re-reads the pointer cell and
 * stores 2 into the object's dispatch byte 0x01, so the next frame runs table
 * entry 2 (0x801F6FD0). The two y words are reached as D_801468C0[...]
 * (0x80146888 + 0x38) with a 38-word, 0x98-byte stride, which keeps the
 * original's folded `lui $at,%hi(D_801468C0)` / `addu $at,$at,$v0` /
 * `lw`|`sw %lo(D_801468C0)($at)` pair for each of the four accesses. The
 * immediately preceding table entry 8 (0x801F763C) opens those same two y
 * words by -0x10000 and +0x10000 and arms byte 0x09 with 0x20, and the 32
 * steps of 0x800 this handler applies return exactly that separation. The
 * countdown-expiry hand-off that stores 2 into the dispatch byte repeats the
 * shape of the still-unlifted entry 1 handler at 0x801F6F24.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Two access spellings carry the shipped bytes. The y word is addressed through
 * a byte-offset base, `(u8 *)D_801468C0 + offset` with a named `s32 offset`, not
 * as a word-indexed `D_801468C0[slot * 38]`: a word-scaled subscript makes GCC
 * materialise the symbol address in its own pseudo, which combine then keeps in
 * a register across all four accesses (`lui $a0,0x8014` / `addiu` once, then
 * `lw`/`sw 0($v0)`), while the byte-offset base stays a `symbol + register`
 * memory address and re-forms the original's `$at` pair per access. Naming the
 * scaled offset in its own local is what removes the remaining shared base
 * (inline `slot * 152` still measured one hoisted base, 176 bytes). The
 * scratchpad pointer cell is read through a named slot-offset local for the same
 * reason: written as a constant-address cell it keeps one hoisted address for
 * the entry load and the expiry re-read, where the original emits `lui` plus an
 * offset load at each read.
 */
void convergeWorkRecordPairYOverCountdownThenAdvanceWorkByte1ScenarioScena1300_801F7958(void) {
  u32 slot_offset;
  u8 *work;
  u8 slot;
  s32 offset;

  slot_offset = 0x44u;
  work = PSX_REF(u8 *, SPAD_BASE + slot_offset);
  if (work[9] != 0) {
    slot = work[3];
    offset = slot * 152;
    *(s32 *)((u8 *)D_801468C0 + offset) += 0x800;
    slot = work[4];
    offset = slot * 152;
    *(s32 *)((u8 *)D_801468C0 + offset) -= 0x800;
    work[9] = work[9] - 1;
  } else {
    work[9] = 0xF;
    PSX_REF(u8 *, SPAD_BASE + slot_offset)[1] = 2;
  }
}
