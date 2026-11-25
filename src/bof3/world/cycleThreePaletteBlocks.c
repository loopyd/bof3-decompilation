#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801F62CC @kind data */
extern u32 paletteSourceRow3Table[4];
/* @source 0x801F62DC @kind data */
extern u32 paletteSourceRow7Table[4];

void copyPaletteStageBlock(u32 sourceRow, u32 sourceBlock,
                                               u32 destinationRow, u32 destinationBlock);

/* @source 0x801F57BC
 * @behavior Copies three 32-byte palette blocks using work byte +9 modulo four
 *           to select source blocks from tables at 0x801F62CC and 0x801F62DC.
 *           Copies row 3 to row 3 block 3, then row 7 to row 6 blocks 9 and 10,
 *           with the last source block one beyond the second. Reloads the work
 *           pointer after each copy and increments work byte +9. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void cycleThreePaletteBlocks(void) {
  u8* work;

  copyPaletteStageBlock(
      3, paletteSourceRow3Table[workRecordCursor1F800044[9] & 3], 3, 3);
  copyPaletteStageBlock(
      7, paletteSourceRow7Table[workRecordCursor1F800044[9] & 3], 6, 9);
  copyPaletteStageBlock(
      7, paletteSourceRow7Table[workRecordCursor1F800044[9] & 3] + 1, 6, 10);
  work = workRecordCursor1F800044;
  work[9]++;
}
