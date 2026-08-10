#include "bof3/ui/game00_internal.h"

/* @behavior clears work-area status bytes then clears work flags */
/* @source 0x8019A7D4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearWorkStatusAndFlags(void) {
  struct GameWorkArea* work = g_game_work;
  work->unk_5F = 0;
  work->unk_5E = 0;
  work->unk_5D = 0;
  clearWorkFlags();
}
