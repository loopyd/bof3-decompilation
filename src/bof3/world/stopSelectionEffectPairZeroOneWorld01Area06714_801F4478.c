#include "bof3/bof3.h"

extern void game_stop_selection_fx(u32 arg0, s32 arg1); /* @source 0x8015D404 */

/* @source 0x801F4478
 * @behavior Thin overlay wrapper published in this overlay's handler table:
 * stops the selection-specific frontend effect pair (group 0, id 1) through
 * the executable-side helper at 0x8015D404, then returns. It reads no overlay
 * cell and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stopSelectionEffectPairZeroOneWorld01Area06714_801F4478(void) {
  game_stop_selection_fx(0u, 1);
}
