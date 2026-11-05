#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;

void copyPaletteBlock32World02Area07714_801F529C(u32 sourceRow, u32 sourceBlock,
                                               u32 destinationRow, u32 destinationBlock);

/* @source 0x801F5710
 * @behavior Copies two 32-byte palette blocks using half of work byte +9:
 *           row 3 block 5+half to row 3 block 3, then row 6 block 11+half
 *           to row 6 block 9. Increments work byte +9, reloads the work
 *           pointer, and advances work byte +2 when byte +9 is at least 6.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void cycleTwoPaletteBlocksAndAdvanceStepWorld02Area07714_801F5710(void) {
  u8* work;

  copyPaletteBlock32World02Area07714_801F529C(
      3, (workRecordCursor1F800044[9] >> 1) + 5, 3, 3);
  copyPaletteBlock32World02Area07714_801F529C(
      6, (workRecordCursor1F800044[9] >> 1) + 11, 6, 9);
  work = workRecordCursor1F800044;
  work[9]++;
  work = workRecordCursor1F800044;
  if (work[9] >= 6) {
    work[2]++;
  }
}
