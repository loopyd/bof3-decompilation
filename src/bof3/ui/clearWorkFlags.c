#include "bof3/ui/game00_internal.h"

/* @behavior clears scratchpad work area bytes at offsets 0x00–0x04.
 * @source 0x80196070
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearWorkFlags(void) {
  g_game_work->flags_00 = 0;
  g_game_work->unk_01 = 0;
  g_game_work->flags_02 = 0;
  g_game_work->pad_03 = 0;
  g_game_work->field_04 = 0;
}
