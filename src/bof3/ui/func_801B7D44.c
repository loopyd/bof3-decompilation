#include "bof3/ui/game00_internal.h"

/* @behavior entry 0 of the work-state table D_801CD330: runs the shared
 * work-area reset service func_801C4D58, raises bit 3 of the world-flags
 * halfword D_8014625A, then advances the scratchpad work area's handler index
 * (g_game_work + 0x04) to the next state, 1.
 * @source 0x801B7D44
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801B7D44(void) {
  func_801C4D58();
  D_8014625A |= 8;
  g_game_work->field_04 = 1;
}
