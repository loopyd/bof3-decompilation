#include "bof3/bof3.h"

/* The overlay's own tables in the resident data blob: two-byte block origins
 * (column byte, then row byte) and the dword fill values selected by the
 * caller's second argument. */
extern u8 D_800C2DC4[];  /* @source 0x800C2DC4 @kind table */
extern u8 D_800C2DC5[];  /* @source 0x800C2DC5 @kind table */
extern u32 D_800C2DC8[]; /* @source 0x800C2DC8 @kind table */

/* @source 0x800C29AC
 * @behavior Writes the fill dword selected by the second argument into every
 * cell of a two-by-two block of the dword cell array at 0x80104030, which is
 * addressed with the resident map row stride byte at 0x80104000. The block
 * origin is the two-byte record of 0x800C2DC4 selected by the first argument:
 * the record's first byte is the block column and its second byte the block
 * row. The original reloads the stride byte, both origin bytes and the fill
 * dword on every inner iteration, so both loops stay as written. Its callers
 * are func_800C1F28, which selects block 0 or 1 from a battle state byte and
 * fill 2, and func_800C2AA4, which passes block 1 with fill 0 or 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fillTwoByTwoCellBlockBossBoss01816_800C29AC(u32 origin, u32 fill) {
  s32 row;
  s32 col;

  for (row = 0; row < 2; row++) {
    for (col = 0; col < 2; col++) {
      u32 index = origin * 2;

      ((u32*)0x80104030)[(D_800C2DC5[index] + row) * *(u8*)0x80104000 +
                         (D_800C2DC4[index] + col)] = D_800C2DC8[fill];
    }
  }
}
