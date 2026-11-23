#include "bof3/world/area03004_internal.h"

/* @source 0x801D4884
 * @behavior scans the 32 entry-amount bytes from index zero, wrapping the work
 * halfword at 0x3E and setting halfword 0x3C when none is available. Draws the
 * fixed header and mode indicators, sets work byte 9 to four and advances state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectFirstAvailableEntry(void) {
  u8 unused_stack[8];
  s32 start;
  s16* work;
  u32 mask;
  u8* mode;
  u8* step;
  u8* state;

  start = 0;
  mode = D_1F800044;
  work = (s16*)mode;
  work[31] = 0;
  work[30] = 0;
  for (;;) {
    s32 index = work[31];
    if (D_80144FE4[index] != 0) {
      goto selected;
    }
    work[31] = index + 1;
    if (work[31] >= 32) {
      work[31] = 0;
    }
    if (work[31] == start) {
      work[30] = 1;
      break;
    }
  }
selected:
  WORLD00_AREA030_DRAW_HEADER_AND_STATUS(mode, mask);
  step = D_1F800044;
  step[9] = 4;
  state = D_1F800044;
  state[1]++;
}
