#include "bof3/bof3.h"

void func_801A4BC0(s32 x, s32 y, u32 size);

/* @source 0x801F369C
 * @behavior Queues the sixteen shared UI elements of the four-by-four block
 *           x = 0x94..0x97, y = 0x20..0x23 through func_801A4BC0 with size 0.
 *           Visits all four x positions for each successive y position.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSharedUiElementBlock94To97x20To23Size0World01Area04013_801F369C(void) {
  s8 y;
  s8 x;

  for (y = 0; y < 4; y++) {
    for (x = 0; x < 4; x++) {
      func_801A4BC0(x + 0x94, y + 0x20, 0);
    }
  }
}
