#include "bof3/bof3.h"

extern u8* D_1F800044;

void func_801F2F44(u32 level);

/* @source 0x801F2EC0
 * @behavior Overlay step handler, entry 4 of the state-byte-+0x02 dispatch
 *           table at 0x801F3930 that the dispatcher 0x801F2C04 indexes: passes
 *           the work record's byte +0x09 shifted right by two as the single
 *           level argument of the local eight-row quad renderer at 0x801F2F44
 *           (whose own body draws eight SetPolyFT4 quads), then decrements byte
 *           +0x09. When the decremented value falls below 8 it reloads byte
 *           +0x09 from the saved byte +0x0A and stores 1 into the record's
 *           dispatch state byte +0x02, handing the next frame to entry 1 of the
 *           same table. The record pointer published by the scratchpad pointer
 *           cell 0x1F800044 is re-read once per group, as the original does.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawCountdownLevelThenRearmFromSavedByteAndSelectStep1World02Area10213_801F2EC0(void) {
  u8* countdown;
  u8* work;

  func_801F2F44(D_1F800044[9] >> 2);
  countdown = D_1F800044;
  countdown[9]--;
  work = D_1F800044;
  if (work[9] < 8) {
    work[9] = work[0xA];
    D_1F800044[2] = 1;
  }
}
