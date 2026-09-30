#include "bof3/ui/game00_internal.h"

/*
 * @behavior runs the shared work-area select service, then publishes state 2
 * into the work-area flags byte at offset 0x02.
 * @source 0x801B32A4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801B32A4(void) {
  func_801C4D1C();
  g_game_work->flags_02 = 2;
}
