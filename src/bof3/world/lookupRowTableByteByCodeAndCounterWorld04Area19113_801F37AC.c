#include "bof3/bof3.h"

extern u8 D_801F4248[]; /* @source 0x801F4248 @kind data */
extern u8 D_801F4250[]; /* @source 0x801F4250 @kind data */
extern u8 D_801F41E8[]; /* @source 0x801F41E8 @kind data */
extern u8 D_801F4218[]; /* @source 0x801F4218 @kind data */
extern u8 D_80181B10[]; /* @source 0x80181B10 @kind table */
extern u8 D_80144982[]; /* @source 0x80144982 @kind data */
extern u8 D_80144F56;   /* @source 0x80144F56 @kind unknown */
extern u32 D_8014686C;  /* @source 0x8014686C @kind data */

s32 func_8015B5D4(u32 arg0, s32 arg1);
u32 lookupTable4260ByteByTable4290OffsetOr8World04Area19113_801F3924(u32 arg0, u32 arg1,
                                                                   u32 arg2, u32 arg3);

/* @source 0x801F37AC
 * @behavior Selects one byte of a five-row table from the caller's code byte
 * and the shared counter D_80144F56. The code is searched in the five-entry
 * row table 0x801F4248, so its position is the row and the row base is that
 * position times the nine-column stride. The counter is read first, clamped to
 * 8, and its own address is then moved down by 0x5E to the main-RAM bank base
 * 0x80144EF8 that the shared wait helper 0x8015B5D4 is asked about with id
 * 0x0A. While that wait reports pending, the answer is the third table
 * 0x801F4260, reached through the sibling lookup 0x801F3924 with the column
 * offset table 0x801F4290. Otherwise the byte at offset 0x1A of the 164-byte
 * field record D_80144982[D_80181B10[code]] picks the column offset: 8 for a
 * record byte of 8 or more, the 0x801F4250 column entry for a record byte
 * below 5 or a counter of 7 or more, and the fixed 7 in between. The same
 * shared wait 0x8015B5D4 then reads the scenario progress word D_8014686C with
 * id 2 and chooses between the 0x801F4218 and 0x801F41E8 rows. Every table
 * access masks the summed index with 0xFF, and the masked index is shared by
 * the two closing table reads. The pending-wait result is truncated to 16 bits
 * at the call, which is the function's own return width.
 * @status partial
 * @match 71.28
 * @residual 67 of 94 instructions and 368 of 376 bytes. The whole body shape
 * and every access, constant, branch and call is accounted for; the single
 * remaining difference is the row base. The original keeps the row base
 * row*9 in its own callee-saved register 0x3814-s4, copies it into the working
 * index 0x3820 and reads the copy back for the fixed 7 and 8 offsets at
 * 0x38A8 and 0x38C4, so its frame saves six registers. Every clean-C shape
 * tried here makes this compiler fold the copy, delete the separate row base
 * and save five, which also moves the loop counter out of that register and
 * shifts the prologue. Fourteen variant shapes (loop form, declaration order,
 * named versus inline product, separate result local, signed and narrower
 * types, mixed accumulation) all measured 67 of 94.
 */
u16 lookupRowTableByteByCodeAndCounterWorld04Area19113_801F37AC(u32 arg0) {
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
    if (D_801F4248[row] == code) {
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
    return (u16)lookupTable4260ByteByTable4290OffsetOr8World04Area19113_801F3924(
        index & 0xFF, code, level, rec);
  }
  if (rec < 8) {
    if (rec < 5 || level >= 7) {
      index += D_801F4250[level];
    } else {
      index = base + 7;
    }
  } else {
    index = base + 8;
  }
  if (func_8015B5D4(D_8014686C, 2) != 0) {
    return D_801F4218[index & 0xFF];
  }
  return D_801F41E8[index & 0xFF];
}
