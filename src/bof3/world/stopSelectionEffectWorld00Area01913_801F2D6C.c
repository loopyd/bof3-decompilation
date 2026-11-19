#include "bof3/bof3.h"

extern void game_stop_selection_fx(u32 arg0, s32 arg1); /* @source 0x8015D404 */

/* @source 0x801F2D6C
 * @behavior Thin overlay wrapper: stops the selection-specific frontend effect
 * pair (group 0, id 0) through the executable-side helper at 0x8015D404, then
 * returns. It reads no overlay cell and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stopSelectionEffectWorld00Area01913_801F2D6C(void) {
  game_stop_selection_fx(0u, 0);
}
