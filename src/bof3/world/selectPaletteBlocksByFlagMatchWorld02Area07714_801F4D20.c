#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801F6278 @kind data */
extern u8 paletteRows801F6278[];
/* @source 0x801F627D @kind data */
extern u8 paletteRows801F627D[];
/* @source 0x80144F28 @kind unknown */
extern s32 D_80144F28;

void copyPaletteBlock32World02Area07714_801F529C(u32 sourceRow, u32 sourceBlock,
                                               u32 destinationRow, u32 destinationBlock);

/* @source 0x801F4D20
 * @behavior Tests the shared word shifted right four against the mask and value
 *           nibbles of work halfword +0x3A. On a match, sets work byte +2 to 3
 *           and selects palette block 9; otherwise increments byte +2 and
 *           selects block 1. Copies that block to block 0 in the row indexed
 *           by signed work halfword +0x36 in table 0x801F6278, then copies
 *           block 8 to block 0 in the row from table 0x801F627D at the reloaded
 *           index. Reloads the work pointer after stores and calls. Takes no
 *           arguments; returns nothing.
 * @status partial
 * @match 90.74
 * @residual Flag shift scheduling, nibble-mask reassociation, condition shift
 *           signedness and test-result register differ.
 */
void selectPaletteBlocksByFlagMatchWorld02Area07714_801F4D20(void) {
  u8* work;
  s32 condition;
  u32 row;
  s32 flags;

  work = workRecordCursor1F800044;
  flags = D_80144F28 >> 4;
  condition = *(u16*)(work + 0x3A);
  if ((((condition >> 4) & 15) & flags) == (condition & 15)) {
    work[2] = 3;
    row = paletteRows801F6278[*(s16*)(workRecordCursor1F800044 + 0x36)];
    copyPaletteBlock32World02Area07714_801F529C(row, 9, row, 0);
  } else {
    work[2]++;
    row = paletteRows801F6278[*(s16*)(workRecordCursor1F800044 + 0x36)];
    copyPaletteBlock32World02Area07714_801F529C(row, 1, row, 0);
  }
  row = paletteRows801F627D[*(s16*)(workRecordCursor1F800044 + 0x36)];
  copyPaletteBlock32World02Area07714_801F529C(row, 8, row, 0);
}
