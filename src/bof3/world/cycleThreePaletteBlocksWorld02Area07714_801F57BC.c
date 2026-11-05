#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801F62CC @kind data */
extern u32 paletteSourceBlocks801F62CC[4];
/* @source 0x801F62DC @kind data */
extern u32 paletteSourceBlocks801F62DC[4];

void copyPaletteBlock32World02Area07714_801F529C(u32 sourceRow, u32 sourceBlock,
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
void cycleThreePaletteBlocksWorld02Area07714_801F57BC(void) {
  u8* work;

  copyPaletteBlock32World02Area07714_801F529C(
      3, paletteSourceBlocks801F62CC[workRecordCursor1F800044[9] & 3], 3, 3);
  copyPaletteBlock32World02Area07714_801F529C(
      7, paletteSourceBlocks801F62DC[workRecordCursor1F800044[9] & 3], 6, 9);
  copyPaletteBlock32World02Area07714_801F529C(
      7, paletteSourceBlocks801F62DC[workRecordCursor1F800044[9] & 3] + 1, 6, 10);
  work = workRecordCursor1F800044;
  work[9]++;
}
