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

/* @source 0x801F4DF8
 * @behavior Copies palette block 1 to block 0 in the row selected by table
 *           0x801F6278 at signed work halfword +0x36, then block 8 to block 0
 *           in the row selected by table 0x801F627D at the reloaded index.
 *           When shared word 0x80144F28 shifted right four and masked by
 *           bits 4..7 of work halfword +0x3A equals its low nibble, clears
 *           work bytes +9/+10 and increments byte +2, reloading the work
 *           pointer after each store. Takes no arguments; returns nothing.
 * @status partial
 * @match 84.31
 * @residual Shared-word load scheduling, nibble-mask placement and test registers differ.
 */
void copyTwoPaletteBlocksAndAdvanceOnFlagMatchWorld02Area07714_801F4DF8(void) {
  u32 row;
  u8* work;
  u32 condition;

  row = paletteRows801F6278[*(s16*)(workRecordCursor1F800044 + 0x36)];
  copyPaletteBlock32World02Area07714_801F529C(row, 1, row, 0);
  row = paletteRows801F627D[*(s16*)(workRecordCursor1F800044 + 0x36)];
  copyPaletteBlock32World02Area07714_801F529C(row, 8, row, 0);
  work = workRecordCursor1F800044;
  condition = *(u16*)(work + 0x3A);
  if (((D_80144F28 >> 4) & ((condition >> 4) & 15)) == (condition & 15)) {
    work[9] = 0;
    workRecordCursor1F800044[10] = 0;
    workRecordCursor1F800044[2]++;
  }
}
