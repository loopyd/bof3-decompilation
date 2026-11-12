#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0, so no target-map row is
 * needed. The sibling bmagic lifts read the same record through this pointer:
 * seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * copies its word triplet, and the magic086 lifts test its byte 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. This target's map binds no name there,
 * so the cell is reached through its address-anchored name and every store
 * group below re-reads it, because a store through the loaded pointer may
 * alias the cell itself. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Motion setup helper of the main executable, called with this overlay's work
 * object and returning nothing. */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 @kind function */

/* Resource request wrapper of the main executable; the sibling bmagic lifts
 * seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * and seedWorkHeaderClearTintGroupAndRequestResourceThenAdvanceWorkByte2
 * BmagicMagic17303_801F0540 declare the same prototype and pass a resource id
 * in the same slot. */
extern void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 @kind function */

/* Resident helper of the main executable this overlay calls with no arguments
 * and with an unfilled branch delay slot, so the call carries no operand. */
extern void func_8014D978(void); /* @source 0x8014D978 @kind function */

/* @source 0x801EF1EC
 * @behavior Per-step countdown then seed of this overlay's work object. It is
 *           word 2 (index 2) of the code-pointer run at 0x801EF6CC that the
 *           lift dispatchStateByte1Tablef6ccBmagicMagic15403_801EF0C0 measured
 *           exact in this target indexes with work-object byte 0x1, the run that
 *           holds 0x801EF104, 0x801EF4B4, this address, 0x801EF380, 0x801EF3E4
 *           and 0x801EF454, so the payload holds no jal to it. It reads the
 *           scratchpad
 *           work-object pointer cell at 0x1F800044 before its frame exists,
 *           decrements the object's countdown byte 0x9 at byte width (lbu /
 *           addiu -1 / sb) and keeps the decremented value in a byte local so
 *           the truncation, not a reload, supplies the compared value (andi
 *           before the test); a non-zero result returns through the shared
 *           epilogue. Only at zero it copies the shared battle work record's
 *           byte 0x8 through 0x801EB4E0 into the object's byte 0x8, clears the
 *           two delta words at 0xC and 0x10, seeds the word at 0x10 with the
 *           object's byte 0xB shifted left 15, and hands the object to the
 *           main-executable motion setup func_801DD350. It then reloads both
 *           pointers and, because that call may have written the two deltas,
 *           reads them back while accumulating the record's words 0x34, 0x38
 *           and 0x3C into the same offsets of the object, seeds the 16.16
 *           motion pair at 0x40/0x44 with 0x8000, sets the mode byte 0x48 to 2
 *           and writes the accumulated 0x3C word last. The header group that
 *           follows reloads the cell for each store, in the order byte 0x25 =
 *           0x1D, byte 0x26 = 0, byte 0x27 = 0xA0, byte 0x28 = 0, byte 0x2A =
 *           bit 0 of byte 0x8, byte 0x29 = 3, byte 0x24 = 4, then the halfword
 *           at 0x2C = 0 and byte 0x2B = 0 from one load. It requests resource 0
 *           through func_8014D6B8, calls func_8014D978, clears countdown byte
 *           0x9 and advances work byte 0x2 with a byte-width read, increment and
 *           store. The frame is 0x18 bytes with the return address at 0x10 and
 *           no saved registers; it takes no arguments and returns nothing.
 *
 *           No byte-identical sibling exists: the closest spans in the tree are
 *           0x198 and 0x1D8 bytes and diverge within their first group, so the
 *           clone lever is declined. The body, one-local-per-store-group form
 *           and cell reload discipline follow the sibling lifts
 *           seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500
 *           _801EF4A4 and seedWorkHeaderClearTintGroupAndRequestResourceThen
 *           AdvanceWorkByte2BmagicMagic17303_801F0540, which had measured exact
 *           in their own targets; this function writes its own literals, its own
 *           store order and its own extra groups.
 * @status partial
 * @match 91.09
 * @residual 92/101 instructions and 404 of 404 bytes: the emitted instruction
 *           sequence, the store order, the size, the frame and every memory
 *           event are the original, and all nine differing instructions are the
 *           register operand of an otherwise identical opcode. This build
 *           colours the battle record pointer into $v1 and the reused word
 *           temporary into $a1, where the original holds them the other way
 *           round, so the first difference at 0x68 is the record pointer's load
 *           and the two delta loads, both sums and the 0x3C load follow it. The
 *           reuse pattern itself is the original's: one word local serves both
 *           record words, one word local serves both deltas, and the 0x3C word is
 *           held across the 0x40/0x44/0x48 group. Measured and reverted
 *           spellings of this group: the two deltas folded inline into their
 *           statements measured 89/101 at 400 bytes with one merged load; a
 *           single reused delta read before the record pointer measured 85/101;
 *           two named sum destinations measured 408 bytes; two temporaries ahead
 *           of the record pointer measured 404 bytes with the motion group moved;
 *           the record pointer re-read at its fixed address measured 420 and 416
 *           bytes; operand order, a second narrow temporary and declaration-order
 *           permutations of these locals measured the same nine-instruction
 *           exchange. The 0x3C copy held in its own word local across the header
 *           group was the change that fixed the instruction order, taking the
 *           body from 89/101 to 92/101 at the original size. The smallest missing
 *           evidence is the register-assignment order of this compiler build for
 *           a pointer that shares one store group with two reused word
 *           temporaries, not a further spelling of the source.
 */
void decrementWorkByte9ThenSeedWorkHeaderAndAdvanceWorkByte2BmagicMagic15403_801EF1EC(
    void) {
  u8 *countdown;
  u8 counter;
  u8 *source;
  u8 *name;
  u8 *work;
  u8 *record;
  s32 sourceWord;
  s32 delta;
  s32 tail;
  u8 *next;
  u8 *tilt;
  u8 *offset;
  u8 *flags;
  u8 *index;
  u8 *mode;
  u8 *kind;
  u8 *pair;
  u8 *clear;
  u8 *step;

  countdown = D_1F800044;
  counter = countdown[9] - 1;
  countdown[9] = counter;
  if (counter != 0) {
    return;
  }
  source = D_801EB4E0;
  name = D_1F800044;
  name[8] = source[8];
  work = D_1F800044;
  *(u32 *)(work + 0xC) = 0;
  *(u32 *)(work + 0x10) = work[0xB] << 15;
  func_801DD350(work);
  record = D_801EB4E0;
  work = D_1F800044;
  sourceWord = *(s32 *)(record + 0x34);
  delta = *(s32 *)(work + 0xC);
  *(s32 *)(work + 0x34) = sourceWord + delta;
  sourceWord = *(s32 *)(record + 0x38);
  delta = *(s32 *)(work + 0x10);
  *(s32 *)(work + 0x38) = sourceWord + delta;
  tail = *(s32 *)(record + 0x3C);
  *(u32 *)(work + 0x40) = 0x8000;
  *(u32 *)(work + 0x44) = 0x8000;
  work[0x48] = 2;
  *(s32 *)(work + 0x3C) = tail;
  next = D_1F800044;
  next[0x25] = 0x1D;
  tilt = D_1F800044;
  tilt[0x26] = 0;
  offset = D_1F800044;
  offset[0x27] = 0xA0;
  kind = D_1F800044;
  kind[0x28] = 0;
  flags = D_1F800044;
  flags[0x2A] = flags[8] & 1;
  index = D_1F800044;
  index[0x29] = 3;
  mode = D_1F800044;
  mode[0x24] = 4;
  pair = D_1F800044;
  *(u16 *)(pair + 0x2C) = 0;
  pair[0x2B] = 0;
  func_8014D6B8(0);
  func_8014D978();
  clear = D_1F800044;
  clear[9] = 0;
  step = D_1F800044;
  step[2]++;
}
