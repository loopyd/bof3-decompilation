#include "bof3/battle/battle15_internal.h"

extern void func_800A82F8(void); /* @source 0x800A82F8 */

/*
 * @source 0x800A5164
 * @behavior Rebuilds the active byte list at D_801463C4 from the three entries
 *   of the four-byte grid cell addressed by the D_801485C8/D_801485CA pair,
 *   keeping every entry that is not the 0xFF sentinel and counting the kept
 *   entries in D_801463C7, then refreshes the list accumulator through
 *   func_800A82F8, stores 2 into D_801485BB, sets bit 2 of local work field
 *   unk_128 and advances the substate byte D_801462E4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void collectGridCellEntriesThenRefreshAccumulator(void) {
  u8 *countAddress;
  u8 *count;
  s16 *pair;
  u8 *counter;
  u8 index;
  u8 value;

  countAddress = &D_801463C7;
  *countAddress = 0;
  index = 0;
  pair = &D_801485C8;
  do {
    value = D_80145500[pair[0] + pair[1]][index];
    if (value != 0xFF) {
      D_801463C4[index] = value;
      count = &D_801463C7;
      *count = *count + 1;
    }
    index++;
  } while (index < 3);
  func_800A82F8();
  D_801485BB = 2;
  D_801EBF08_PTR->unk_128 |= 4;
  counter = &D_801462E4;
  *counter = *counter + 1;
}
