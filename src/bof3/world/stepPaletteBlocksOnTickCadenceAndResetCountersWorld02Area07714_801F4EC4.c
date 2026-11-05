#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801F6278 @kind data */
extern u8 paletteRows801F6278[];
/* @source 0x801F627D @kind data */
extern u8 paletteRows801F627D[];

void copyPaletteBlock32World02Area07714_801F529C(u32 sourceRow, u32 sourceBlock,
                                               u32 destinationRow, u32 destinationBlock);

/* @source 0x801F4EC4
 * @behavior Reads the non-volatile scratchpad work pointer cell 0x1F800044.
 *           Splits work byte +9 into five-tick phases; when the remainder is
 *           zero, copies palette block eight minus the phase quotient to block
 *           zero of the row selected by table 0x801F627D at signed work
 *           halfword +0x36. Increments byte +9, then copies block work byte
 *           +10 plus one to block zero of the row from table 0x801F6278 while
 *           byte +10 is below nine. Increments byte +10; when byte +9 has
 *           reached 0x1F, clears bytes +9 and +10 and increments work byte +2.
 *           Reloads the work pointer after stores and calls. Takes no
 *           arguments; returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepPaletteBlocksOnTickCadenceAndResetCountersWorld02Area07714_801F4EC4(void) {
  u8* work;
  u8* tickCursor;
  u8* blockCursor;
  u8* phaseCursor;
  u8* cycleCursor;
  u8* resetCursor;
  u8* stateCursor;
  u32 ticks;
  u8 row;

  work = workRecordCursor1F800044;
  ticks = work[9];
  if ((u8)(ticks % 5) == 0) {
    row = paletteRows801F627D[*(s16*)(work + 0x36)];
    copyPaletteBlock32World02Area07714_801F529C(row, 8 - (u8)(ticks / 5), row, 0);
  }
  tickCursor = workRecordCursor1F800044;
  tickCursor[9]++;
  blockCursor = workRecordCursor1F800044;
  if (blockCursor[10] < 9) {
    row = paletteRows801F6278[*(s16*)(blockCursor + 0x36)];
    copyPaletteBlock32World02Area07714_801F529C(row, blockCursor[10] + 1, row, 0);
  }
  phaseCursor = workRecordCursor1F800044;
  phaseCursor[10]++;
  cycleCursor = workRecordCursor1F800044;
  if (cycleCursor[9] >= 0x1F) {
    cycleCursor[9] = 0;
    resetCursor = workRecordCursor1F800044;
    resetCursor[10] = 0;
    stateCursor = workRecordCursor1F800044;
    stateCursor[2]++;
  }
}
