#include "bof3/bof3.h"

extern u8 D_801F3878[]; /* @source 0x801F3878 @kind table */
extern u8 D_801F3880[]; /* @source 0x801F3880 @kind table */
extern u8 D_801F3818[]; /* @source 0x801F3818 @kind table */
extern u8 D_801F3848[]; /* @source 0x801F3848 @kind table */
extern u8 D_80181B10[]; /* @source 0x80181B10 @kind table */
extern u8 D_80144982[]; /* @source 0x80144982 @kind data */
extern u8 D_80144F56;   /* @source 0x80144F56 @kind unknown */
extern u32 D_8014686C;  /* @source 0x8014686C @kind data */

s32 func_8015B5D4(u32 arg0, s32 arg1);
u8 lookupTable3890ByteByTable38C0OffsetOr8World04Area19213_801F33E4(u32 arg0,
                                                                    u32 arg1,
                                                                    u32 arg2,
                                                                    u32 arg3);

/* @source 0x801F326C
 * @behavior Selects one byte of a five-row table from the caller's code byte
 * and the shared counter D_80144F56. The code is searched in the five-entry row
 * table 0x801F3878, so its position is the row and the row base is that
 * position times the nine-column stride. The counter is read first, clamped to
 * 8, and its own address is then moved down by 0x5E to the main-RAM bank base
 * 0x80144EF8 that the shared wait helper 0x8015B5D4 is asked about with id
 * 0x0A. While that wait reports pending, the answer is the third table
 * 0x801F33E4 lookup with the column offset table 0x801F38C0. Otherwise the
 * byte at offset 0x1A of the 164-byte
 * field record addressed as D_80144982[D_80181B10[code]] picks the column
 * offset: 8 for a record byte of 8 or more, the 0x801F3880 column entry for a
 * record byte below 5 or a counter of 7 or more, and the fixed 7 in between.
 * The same shared wait 0x8015B5D4 then reads the scenario progress word
 * D_8014686C with id 2 and chooses between the 0x801F3848 and 0x801F3818 rows.
 * Every table access masks the summed index with 0xFF, and the masked index is
 * shared by the two closing table reads. The pending-wait result is truncated
 * to 16 bits at the call, which is the function's own return width.
 * @status partial
 * @match 70.21
 * @residual 66 of 94 instructions and 368 of 376 bytes. The whole body shape
 * and every access, constant, branch and call is accounted for; the single
 * remaining difference is that the original keeps the row base row*9 in its
 * own callee-saved register $s4 and copies it into the working index $s0, whose
 * value it re-reads at the pending-wait argument and accumulates for the 0x801F3880
 * column entry, so its frame saves six registers. Every clean-C shape tried here
 * makes this compiler fold the copy, delete the separate row base and save five,
 * which shifts the prologue and every later instruction. Fifteen variants (copy in
 * either direction, chained assignment, declaration order, copy after the clamp,
 * after the record load and after the bank move, base computed before the counter
 * load, CSE-shaped double product, table arm reading the row base, repeated row*9
 * expression in the fixed-offset arms, the dead formal reused as the index and an
 * initialized index) all measured 66 of 94 with first difference at byte 0x0004.
 *
 * Name evidence: the searched five-entry row table 0x801F3878, the 0x801F3880
 * column entry and the two 0x801F3818/0x801F3848 five-row tables selected by the
 * D_8014686C progress word are the displacements the original folds into its
 * `lbu` loads; the id 0x0A wait on the base 0x80144EF8 and the id 2 wait on
 * D_8014686C are the `addiu $a1, $zero` immediates in the two 0x8015B5D4 call
 * sequences; and the row scan over 0x801F3878 is the loop the body opens with.
 * The body is the same shape as the lifted sibling at emi/world04/area191/13
 * 0x801F37AC: all 94 instruction words of the two ranges agree apart from the
 * table, bank and call displacements, so the wording above follows that
 * sibling's record. That sibling lives in another target and differs in those
 * relocations, so it is shape evidence and not a clone candidate; the range
 * carries its own prologue and `jr $ra` epilogue at the payload's function
 * boundary, so the carve lever does not apply either.
 */
u16 lookupRowTableByteByCodeAndCounterWorld04Area19213_801F326C(u32 arg0) {
  u8 *bank;
  u8 code;
  u8 row;
  u8 level;
  u8 rec;
  u32 base;
  u32 index;

  code = arg0 & 0xFF;
  row = 0;
  while (row < 5) {
    if (D_801F3878[row] == code) {
      break;
    }
    row++;
  }
  bank = &D_80144F56;
  level = *bank;
  base = row * 9;
  index = base;
  if (level > 8) {
    level = 8;
  }
  bank -= 0x5E;
  rec = D_80144982[D_80181B10[code] * 164];
  if (func_8015B5D4((u32)bank, 0x0A) != 0) {
    return (u16)lookupTable3890ByteByTable38C0OffsetOr8World04Area19213_801F33E4(
        index & 0xFF, code, level, rec);
  }
  if (rec < 8) {
    if (rec < 5 || level >= 7) {
      index += D_801F3880[level];
    } else {
      index = base + 7;
    }
  } else {
    index = base + 8;
  }
  if (func_8015B5D4(D_8014686C, 2) != 0) {
    return D_801F3848[index & 0xFF];
  }
  return D_801F3818[index & 0xFF];
}
