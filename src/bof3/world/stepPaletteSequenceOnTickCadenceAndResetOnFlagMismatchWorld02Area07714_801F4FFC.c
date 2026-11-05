#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801F6278 @kind data */
extern u8 paletteRows801F6278[];
/* @source 0x801F627D @kind data */
extern u8 paletteRows801F627D[];
/* @source 0x801F6298 @kind data */
extern u8 paletteSourceBlocks801F6298[];
/* @source 0x80144F28 @kind unknown */
extern s32 D_80144F28;

void copyPaletteBlock32World02Area07714_801F529C(u32 sourceRow, u32 sourceBlock,
                                               u32 destinationRow, u32 destinationBlock);

/* @source 0x801F4FFC
 * @behavior Reads the non-volatile scratchpad work pointer cell 0x1F800044.
 *           When work byte +9 is a multiple of four, copies palette block
 *           paletteSourceBlocks801F6298[(byte +9 >> 2) % 6] to block zero of
 *           the row selected by table 0x801F627D at signed work halfword +0x36.
 *           Copies fixed block nine to block zero of the row from table
 *           0x801F6278 at the reloaded index. When shared word 0x80144F28
 *           shifted right four and masked by bits 4..7 of work halfword +0x3A
 *           differs from its low nibble and (byte +9 >> 2) % 6 is zero, clears
 *           bytes +9 and +10 and increments byte +2, reloading the work pointer
 *           after each store. Finally reloads the work pointer and increments
 *           byte +9. Takes no arguments; returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepPaletteSequenceOnTickCadenceAndResetOnFlagMismatchWorld02Area07714_801F4FFC(
    void) {
  u8* work;
  u8* blockCursor;
  u8* flagCursor;
  u8* resetCursor;
  u8* stepCursor;
  u8* tickCursor;
  u32 ticks;
  u8 row;
  s32 flags;
  u32 condition;
  u32 mask;

  work = workRecordCursor1F800044;
  ticks = work[9];
  if ((ticks & 3) == 0) {
    row = paletteRows801F627D[*(s16*)(work + 0x36)];
    copyPaletteBlock32World02Area07714_801F529C(
        row, paletteSourceBlocks801F6298[(u8)((ticks >> 2) % 6)], row, 0);
  }
  blockCursor = workRecordCursor1F800044;
  row = paletteRows801F6278[*(s16*)(blockCursor + 0x36)];
  copyPaletteBlock32World02Area07714_801F529C(row, 9, row, 0);
  flagCursor = workRecordCursor1F800044;
  flags = D_80144F28;
  condition = *(u16*)(flagCursor + 0x3A);
  mask = (condition >> 4) & 15;
  flags = (flags >> 4) & mask;
  if ((u32)flags != (condition & 15)) {
    flags = (u8)((flagCursor[9] >> 2) % 6);
    if (flags == 0) {
      flagCursor[9] = 0;
      resetCursor = workRecordCursor1F800044;
      resetCursor[10] = 0;
      stepCursor = workRecordCursor1F800044;
      stepCursor[2]++;
    }
  }
  tickCursor = workRecordCursor1F800044;
  tickCursor[9]++;
}
