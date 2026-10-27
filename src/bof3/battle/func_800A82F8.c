#include "bof3/battle/battle15_internal.h"

/* @source 0x800A82F8
 * @behavior Zeroes the byte accumulator at D_801463B8, then adds the mapped
 *   value of every active list byte into it, once per entry of the active
 *   byte count.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A82F8(void)
{
  u8 listIndex;
  u32 entryCount;
  u8 *accumulatorAddress;
  u8 *accumulator;

  listIndex = 0;
  entryCount = D_801463C7;
  accumulatorAddress = &D_801463B8;
  *accumulatorAddress = 0;
  if (entryCount != 0) {
    accumulator = accumulatorAddress;
    do {
      *accumulator += byteValueTable[D_801463C4[listIndex]];
      listIndex++;
    } while (listIndex < D_801463C7);
  }
}
