#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801F6278 @kind data */
extern u8 paletteRows801F6278[];
/* @source 0x801F627D @kind data */
extern u8 paletteRows801F627D[];
/* @source 0x801F62A0 @kind data */
extern u8 paletteSourceBlocks801F62A0[];

void copyPaletteBlock32World02Area07714_801F529C(u32 sourceRow, u32 sourceBlock,
                                               u32 destinationRow, u32 destinationBlock);

/* @source 0x801F515C
 * @behavior Reads the non-volatile scratchpad work pointer cell 0x1F800044.
 *           Splits work byte +9 into five-tick phases; when the remainder is
 *           zero, copies palette block (byte +9 / 5) + 2 to block zero of the
 *           row selected by table 0x801F627D at signed work halfword +0x36.
 *           Increments byte +9, then copies the source block taken from the
 *           five-byte block list 0x801F62A0 at byte +10 halved to block zero
 *           of the row from table 0x801F6278 while byte +10 is even and below
 *           nine. Increments byte +10; when byte +9 has reached 0x1F, clears
 *           bytes +9 and +10 and sets work byte +2 to 1. Reloads the work
 *           pointer after stores and calls. Takes no arguments; returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepRampedAndTableSelectedPaletteBlocksOnTickCadenceWorld02Area07714_801F515C(
    void) {
  u8* work;
  u8* tickCursor;
  u8* blockCursor;
  u8* phaseCursor;
  u8* cycleCursor;
  u8* resetCursor;
  u8* stateCursor;
  u32 ticks;
  u32 block;
  u32 count;
  u8 row;

  work = workRecordCursor1F800044;
  ticks = work[9];
  if ((u8)(ticks % 5) == 0) {
    row = paletteRows801F627D[*(s16*)(work + 0x36)];
    copyPaletteBlock32World02Area07714_801F529C(row, (u8)(ticks / 5) + 2, row,
                                                0);
  }
  tickCursor = workRecordCursor1F800044;
  tickCursor[9]++;
  blockCursor = workRecordCursor1F800044;
  block = blockCursor[10];
  if ((block & 1) == 0) {
    count = (u8)block;
    if (count < 9) {
      row = paletteRows801F6278[*(s16*)(blockCursor + 0x36)];
      copyPaletteBlock32World02Area07714_801F529C(
          row, paletteSourceBlocks801F62A0[count >> 1], row, 0);
    }
  }
  phaseCursor = workRecordCursor1F800044;
  phaseCursor[10]++;
  cycleCursor = workRecordCursor1F800044;
  if (cycleCursor[9] >= 0x1F) {
    cycleCursor[9] = 0;
    resetCursor = workRecordCursor1F800044;
    resetCursor[10] = 0;
    stateCursor = workRecordCursor1F800044;
    stateCursor[2] = 1;
  }
}
