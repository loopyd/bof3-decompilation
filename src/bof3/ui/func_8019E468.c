#include "bof3/ui/game00_internal.h"

/* @behavior clears work byte 9, sets byte 0xA to 5, then increments work flags byte 2 */
/* @source 0x8019E468
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8019E468(void) {
  g_game_work->pad_09[0] = 0;
  g_game_work->pad_09[1] = 5;
  g_game_work->flags_02 += 1;
}
