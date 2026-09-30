#include "bof3/ui/game00_internal.h"

/* @behavior runs the shared work-area scratchpad service func_801C51B0, then
 * advances the work-area byte at offset 0x03 (the handler index of the
 * D_801CD140/D_801CD154 dispatch tables) to 1.
 * @source 0x801B3584
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801B3584(void) {
  func_801C51B0();
  g_game_work->pad_03 = 1;
}
