#include "bof3/world/area03004_internal.h"

/* @source 0x801D4884
 * @behavior scans the 32 entry-amount bytes from index zero, wrapping the work
 * halfword at 0x3E and setting halfword 0x3C when none is available. Draws the
 * fixed header and mode indicators, sets work byte 9 to four and advances state.
 * @status partial
 * @match 76.56
 * @residual entry constant/pointer allocation and 32-byte frame differ from
 * the original 40-byte frame
 */
void selectFirstAvailableEntryAndAdvanceStateWorld00Area03004_801D4884(void) {
  s32 start;
  s16* work;
  u32 mask;
  u8* mode;
  u8* step;
  u8* state;

  start = 0;
  work = (s16*)D_1F800044;
  work[31] = 0;
  work[30] = 0;
  do {
    s32 index = work[31];
    if (D_80144FE4[index] != 0) {
      goto selected;
    }
    work[31] = index + 1;
    if (work[31] >= 32) {
      work[31] = 0;
    }
  } while (work[31] != start);
  work[30] = 1;
selected:
  func_801D9534(20, 18, 280, 19, 0);
  mask = 0;
  if (D_80145E93 >= 3) {
    mode = D_1F800044;
    mask = (u8)(1u << mode[6]);
  }
  drawLabeledStatusCellsWorld00Area03004_801D7C10(88, 40, mask);
  step = D_1F800044;
  step[9] = 4;
  state = D_1F800044;
  state[1]++;
}
